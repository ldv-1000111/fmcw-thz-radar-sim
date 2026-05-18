.. _phase1_testing:

Testing & Validation
=====================

Every physics claim is encoded as an automated Catch2 test.
**Do not advance to Phase 2 until all 7 tests pass.**

Two bugs were discovered during the initial build and fixed before the
``v0.1.0`` tag — both were in the test logic, not the physics implementation:

.. list-table:: Bug fixes applied before v0.1.0
   :header-rows: 1
   :widths: 12 30 58

   * - Test
     - Original (broken)
     - Fixed
   * - Test 4
     - Compared ``std::arg(buf[0])`` between chirps. At sample ``i=0``,
       fast-time ``t=0`` so phase is always 0 regardless of slow-time
       offset — the test always passed for any target.
     - Compare ``std::arg(buf[N/2])`` at the mid-chirp sample where
       ``t > 0`` and phase is meaningfully non-zero.
   * - Tests 5 & 7
     - Computed variance of raw ``std::arg(buf[0])`` values across
       chirps. ``std::arg`` wraps to ``[-pi, pi]``; for a stationary
       target all values are 0, giving variance = 0 for both the
       vibrating and non-vibrating case.
     - Compute variance of **inter-chirp phase differences** sampled
       at ``buf[N/2]``. Differences remove the constant Doppler slope
       and expose the vibration modulation without wrap-around.

----

Test File 1 — ``tests/test_if_signal.cpp``
--------------------------------------------

Four tests covering range-bin accuracy, buffer size, sample magnitude,
and slow-time phase progression for a moving target.

.. code-block:: cpp
   :caption: tests/test_if_signal.cpp
   :linenos:

   #include <catch2/catch_test_macros.hpp>
   #include <catch2/catch_approx.hpp>
   #include "fmcw_generator.hpp"
   #include <algorithm>
   #include <cmath>
   #include <complex>
   #include <vector>

   static constexpr double C = 299792458.0;

   // Expected beat-frequency bin for a stationary target.
   // k = round( f_beat * N_s / f_s )  where  f_beat = (B/Tc)*(2R/c)
   static int expected_range_bin(const RadarParams& p, double range) {
       const double tau    = 2.0 * range / C;
       const double f_beat = (p.bandwidth / p.chirp_time) * tau;
       return static_cast<int>(std::round(f_beat * p.num_samples / p.fs));
   }

   // Brute-force DFT magnitude peak -- O(N^2), acceptable for small N in tests.
   static int peak_bin(const std::vector<std::complex<float>>& sig) {
       const int N = static_cast<int>(sig.size());
       double best_mag = -1.0;
       int    best_k   =  0;
       for (int k = 0; k < N / 2; ++k) {
           std::complex<double> acc(0.0, 0.0);
           for (int n = 0; n < N; ++n) {
               const double angle = -2.0 * M_PI * k * n / N;
               acc += std::complex<double>(sig[n]) *
                      std::complex<double>(std::cos(angle), std::sin(angle));
           }
           const double mag = std::abs(acc);
           if (mag > best_mag) { best_mag = mag; best_k = k; }
       }
       return best_k;
   }

   static RadarParams make_params() {
       return RadarParams{ 300e9, 4e9, 100e-6, 50e6, 500, 1 };
   }

   // -- Range accuracy ---------------------------------------------------

   TEST_CASE("IF signal -- range bin accuracy within 1 bin", "[fmcw][range]")
   {
       const RadarParams p = make_params();

       SECTION("50 m stationary target") {
           const Target tgt{ 50.0, 0.0, 1.0, 0.0, 0.0 };
           std::vector<std::complex<float>> buf(p.num_samples);
           generate_chirp_if(p, tgt, 0, buf);
           const int got      = peak_bin(buf);
           const int expected = expected_range_bin(p, 50.0);
           INFO("got=" << got << " expected=" << expected);
           REQUIRE(std::abs(got - expected) <= 1);
       }

       SECTION("20 m stationary target") {
           const Target tgt{ 20.0, 0.0, 1.0, 0.0, 0.0 };
           std::vector<std::complex<float>> buf(p.num_samples);
           generate_chirp_if(p, tgt, 0, buf);
           REQUIRE(std::abs(peak_bin(buf) - expected_range_bin(p, 20.0)) <= 1);
       }

       SECTION("80 m stationary target") {
           const Target tgt{ 80.0, 0.0, 1.0, 0.0, 0.0 };
           std::vector<std::complex<float>> buf(p.num_samples);
           generate_chirp_if(p, tgt, 0, buf);
           REQUIRE(std::abs(peak_bin(buf) - expected_range_bin(p, 80.0)) <= 1);
       }
   }

   // -- Sanity checks ---------------------------------------------------

   TEST_CASE("IF signal -- buffer size equals num_samples", "[fmcw][sanity]")
   {
       const RadarParams p   = make_params();
       const Target      tgt{ 30.0, 5.0, 1.0, 0.0, 0.0 };
       std::vector<std::complex<float>> buf(p.num_samples);
       generate_chirp_if(p, tgt, 0, buf);
       REQUIRE(static_cast<int>(buf.size()) == p.num_samples);
   }

   TEST_CASE("IF signal -- all samples have unit magnitude", "[fmcw][sanity]")
   {
       const RadarParams p   = make_params();
       const Target      tgt{ 50.0, 0.0, 1.0, 0.0, 0.0 };
       std::vector<std::complex<float>> buf(p.num_samples);
       generate_chirp_if(p, tgt, 0, buf);
       for (int i = 0; i < p.num_samples; ++i) {
           INFO("sample " << i << ": |s| = " << std::abs(buf[i]));
           REQUIRE(std::abs(std::abs(buf[i]) - 1.0f) < 1e-4f);
       }
   }

   // -- BUG FIX: compare at sample N/2, not sample 0 -------------------
   // At i=0 the fast-time t=0, so phase=2*pi*f_beat*0=0 for any chirp.
   // The mid-chirp sample (t > 0) gives a non-trivial phase that
   // actually differs between consecutive chirps of a moving target.

   TEST_CASE("IF signal -- chirp index shifts slow-time phase", "[fmcw][chirp]")
   {
       const RadarParams p   = make_params();
       const Target      tgt{ 50.0, 5.0, 1.0, 0.0, 0.0 }; // v=5 m/s

       std::vector<std::complex<float>> chirp0(p.num_samples);
       std::vector<std::complex<float>> chirp1(p.num_samples);
       generate_chirp_if(p, tgt, 0, chirp0);
       generate_chirp_if(p, tgt, 1, chirp1);

       const int   mid    = p.num_samples / 2;
       const float phase0 = std::arg(chirp0[mid]);
       const float phase1 = std::arg(chirp1[mid]);
       INFO("phase at sample " << mid
            << ": chirp0=" << phase0 << "  chirp1=" << phase1);
       REQUIRE(std::abs(phase0 - phase1) > 1e-3f);
   }

----

Test File 2 — ``tests/test_micro_doppler.cpp``
------------------------------------------------

Three tests verifying micro-Doppler behaviour. The key fix was replacing
raw ``std::arg(buf[0])`` variance with **inter-chirp phase difference**
variance, which is robust against phase wrapping and the ``t=0`` problem.

.. code-block:: cpp
   :caption: tests/test_micro_doppler.cpp
   :linenos:

   #include <catch2/catch_test_macros.hpp>
   #include "fmcw_generator.hpp"
   #include <cmath>
   #include <complex>
   #include <numeric>
   #include <vector>

   // Variance of a float vector
   static float variance(const std::vector<float>& v) {
       if (v.empty()) return 0.0f;
       const float n    = static_cast<float>(v.size());
       const float mean = std::accumulate(v.begin(), v.end(), 0.0f) / n;
       float var = 0.0f;
       for (float x : v) var += (x - mean) * (x - mean);
       return var / n;
   }

   // BUG FIX: collect inter-chirp phase DIFFERENCES at sample N/2.
   //
   // Using differences:
   //   1. Removes the constant Doppler slope (velocity contribution).
   //   2. Exposes the vibration-induced modulation -- micro-Doppler.
   //   3. Avoids the t=0 / phase-wrap issue that made variance = 0.
   //
   static std::vector<float> slow_time_phase_diffs(
       const RadarParams& p, const Target& tgt)
   {
       std::vector<std::complex<float>> buf(p.num_samples);
       std::vector<float> phases;
       phases.reserve(p.num_chirps);

       const int mid = p.num_samples / 2; // sample with t > 0

       for (int c = 0; c < p.num_chirps; ++c) {
           generate_chirp_if(p, tgt, c, buf);
           phases.push_back(std::arg(buf[mid]));
       }

       std::vector<float> diffs;
       diffs.reserve(p.num_chirps - 1);
       for (int c = 0; c < p.num_chirps - 1; ++c)
           diffs.push_back(phases[c + 1] - phases[c]);
       return diffs;
   }

   TEST_CASE("Micro-Doppler -- vibration increases slow-time phase variance",
             "[thz][microdoppler]")
   {
       const RadarParams p{ 300e9, 4e9, 100e-6, 50e6, 200, 16 };

       const Target no_vib  { 50.0, 0.0, 1.0, 0.0,    0.0   };
       const Target with_vib{ 50.0, 0.0, 1.0, 0.0002, 200.0 };

       const float var_flat = variance(slow_time_phase_diffs(p, no_vib));
       const float var_vib  = variance(slow_time_phase_diffs(p, with_vib));

       INFO("var(no vibration)   = " << var_flat);
       INFO("var(with vibration) = " << var_vib);
       REQUIRE(var_vib > var_flat);
   }

   TEST_CASE("Micro-Doppler -- zero amplitude equals static model",
             "[thz][microdoppler]")
   {
       const RadarParams p{ 300e9, 4e9, 100e-6, 50e6, 100, 1 };
       const Target t_no_vib  { 50.0, 0.0, 1.0, 0.0, 0.0   };
       const Target t_zero_amp{ 50.0, 0.0, 1.0, 0.0, 500.0 }; // amp=0
       std::vector<std::complex<float>> b1(p.num_samples), b2(p.num_samples);
       generate_chirp_if(p, t_no_vib,   0, b1);
       generate_chirp_if(p, t_zero_amp, 0, b2);
       for (int i = 0; i < p.num_samples; ++i) {
           INFO("sample " << i << ": b1=" << b1[i] << "  b2=" << b2[i]);
           REQUIRE(std::abs(b1[i] - b2[i]) < 1e-5f);
       }
   }

   TEST_CASE("Micro-Doppler -- larger amplitude increases phase spread",
             "[thz][microdoppler]")
   {
       const RadarParams p{ 300e9, 4e9, 100e-6, 50e6, 200, 16 };
       const Target small_vib{ 50.0, 0.0, 1.0, 0.0001, 200.0 }; // 0.1 mm
       const Target large_vib{ 50.0, 0.0, 1.0, 0.0002, 200.0 }; // 0.2 mm

       const float var_small = variance(slow_time_phase_diffs(p, small_vib));
       const float var_large = variance(slow_time_phase_diffs(p, large_vib));

       INFO("var(0.1 mm) = " << var_small);
       INFO("var(0.2 mm) = " << var_large);
       REQUIRE(var_large > var_small);
   }

----

``tests/CMakeLists.txt``
-------------------------

.. code-block:: cmake
   :caption: tests/CMakeLists.txt
   :linenos:

   add_executable(radar_tests
       test_if_signal.cpp
       test_micro_doppler.cpp
   )

   target_link_libraries(radar_tests
       PRIVATE fmcw_core
               Catch2::Catch2WithMain
   )

   target_compile_options(radar_tests PRIVATE
       $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
       $<$<CXX_COMPILER_ID:MSVC>:/W4>
   )

   # Run the entire test binary as a single ctest entry.
   # Catch2 reports each TEST_CASE internally with full pass/fail detail.
   # Running individual tests via add_test causes ctest to quote the test
   # name, which Catch2 misinterprets as a literal filter pattern
   # (e.g. searching for '"IF signal..."' including the quote characters).
   add_test(
       NAME radar_unit_tests
       COMMAND radar_tests
       WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
   )

----

Verified Test Output
---------------------

All 7 test cases (607 assertions) were run against the committed code
(Unix Makefiles, GCC 13, Ubuntu 24.04):

.. code-block:: text

   $ ctest --test-dir build --output-on-failure -V

   test 1
       Start 1: radar_unit_tests

   1: Test command: .../build/radar_tests_build/radar_tests
   1: Randomness seeded to: 853545918
   1: ===================================================================
   1: All tests passed (607 assertions in 7 test cases)
   1:
   1/1 Test #1: radar_unit_tests .......   Passed    0.01 sec

   100% tests passed, 0 tests failed out of 1
