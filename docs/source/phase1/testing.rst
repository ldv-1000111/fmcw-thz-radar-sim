.. _phase1_testing:

Testing & Validation
=====================

Every physics claim is encoded as an automated Catch2 test.
**Do not advance to Phase 2 until all 7 tests pass.**

Test File 1 — Range Accuracy
------------------------------

Validates that ``generate_chirp_if()`` places the FFT peak at the correct
range bin for three different target distances, and that the buffer size
and sample magnitude are correct.

.. code-block:: cpp
   :caption: tests/test_if_signal.cpp
   :linenos:

   #include <catch2/catch_test_macros.hpp>
   #include <catch2/catch_approx.hpp>
   #include "fmcw_generator.hpp"
   #include <algorithm>
   #include <cmath>

   static constexpr double C = 299792458.0;

   // Expected beat-frequency bin for a stationary target
   static int expected_range_bin(const RadarParams& p, double range) {
       double tau    = 2.0 * range / C;
       double f_beat = (p.bandwidth / p.chirp_time) * tau;
       return static_cast<int>(std::round(f_beat * p.num_samples / p.fs));
   }

   // Index of the magnitude peak (brute-force DFT, small N only)
   static int peak_bin(const std::vector<std::complex<float>>& sig) {
       int N = static_cast<int>(sig.size());
       std::vector<double> mag(N / 2, 0.0);
       for (int k = 0; k < N / 2; ++k) {
           std::complex<double> acc = 0;
           for (int n = 0; n < N; ++n)
               acc += std::complex<double>(sig[n]) *
                      std::polar(1.0, -2.0 * M_PI * k * n / N);
           mag[k] = std::abs(acc);
       }
       return static_cast<int>(
           std::max_element(mag.begin(), mag.end()) - mag.begin());
   }

   TEST_CASE("IF signal — range bin accuracy within 1 bin", "[fmcw][range]") {
       RadarParams p { 300e9, 4e9, 100e-6, 50e6, 500, 1 };

       SECTION("50 m stationary target") {
           Target tgt { 50.0, 0.0, 1.0, 0.0, 0.0 };
           std::vector<std::complex<float>> buf(p.num_samples);
           generate_chirp_if(p, tgt, 0, buf);
           int got      = peak_bin(buf);
           int expected = expected_range_bin(p, 50.0);
           REQUIRE(std::abs(got - expected) <= 1);
       }

       SECTION("20 m stationary target") {
           Target tgt { 20.0, 0.0, 1.0, 0.0, 0.0 };
           std::vector<std::complex<float>> buf(p.num_samples);
           generate_chirp_if(p, tgt, 0, buf);
           REQUIRE(std::abs(peak_bin(buf) - expected_range_bin(p, 20.0)) <= 1);
       }

       SECTION("80 m stationary target") {
           Target tgt { 80.0, 0.0, 1.0, 0.0, 0.0 };
           std::vector<std::complex<float>> buf(p.num_samples);
           generate_chirp_if(p, tgt, 0, buf);
           REQUIRE(std::abs(peak_bin(buf) - expected_range_bin(p, 80.0)) <= 1);
       }
   }

   TEST_CASE("IF signal — buffer size equals num_samples", "[fmcw][sanity]") {
       RadarParams p { 300e9, 4e9, 100e-6, 50e6, 500, 1 };
       Target tgt { 30.0, 5.0, 1.0, 0.0, 0.0 };
       std::vector<std::complex<float>> buf(p.num_samples);
       generate_chirp_if(p, tgt, 0, buf);
       REQUIRE(static_cast<int>(buf.size()) == p.num_samples);
   }

   TEST_CASE("IF signal — all samples have unit magnitude", "[fmcw][sanity]") {
       RadarParams p { 300e9, 4e9, 100e-6, 50e6, 200, 1 };
       Target tgt { 50.0, 0.0, 1.0, 0.0, 0.0 };
       std::vector<std::complex<float>> buf(p.num_samples);
       generate_chirp_if(p, tgt, 0, buf);
       for (auto& s : buf)
           REQUIRE(std::abs(std::abs(s) - 1.0f) < 1e-4f);
   }

Test File 2 — Micro-Doppler
-----------------------------

Validates that enabling the vibration model increases slow-time phase
variance (a proxy for Doppler sideband energy), and that setting
``vib_amp = 0`` produces identical output to no vibration.

.. code-block:: cpp
   :caption: tests/test_micro_doppler.cpp
   :linenos:

   #include <catch2/catch_test_macros.hpp>
   #include "fmcw_generator.hpp"
   #include <cmath>
   #include <vector>

   TEST_CASE("Micro-Doppler — vibration increases slow-time phase variance",
             "[thz][microdoppler]") {
       RadarParams p { 300e9, 4e9, 100e-6, 50e6, 200, 8 };

       auto collect_phases = [&](double amp, double freq) {
           Target tgt { 50.0, 0.0, 1.0, amp, freq };
           std::vector<float> phases;
           std::vector<std::complex<float>> buf(p.num_samples);
           for (int c = 0; c < p.num_chirps; ++c) {
               generate_chirp_if(p, tgt, c, buf);
               phases.push_back(std::arg(buf[0]));
           }
           return phases;
       };

       auto no_vib   = collect_phases(0.0,    0.0);
       auto with_vib = collect_phases(0.0002, 200.0);

       auto phase_var = [](const std::vector<float>& v) {
           float mean = 0;
           for (float x : v) mean += x;
           mean /= static_cast<float>(v.size());
           float var = 0;
           for (float x : v) var += (x - mean) * (x - mean);
           return var / static_cast<float>(v.size());
       };

       // Vibrating target must show strictly higher phase variance
       REQUIRE(phase_var(with_vib) > phase_var(no_vib));
   }

   TEST_CASE("Micro-Doppler — zero amplitude equals static model",
             "[thz][microdoppler]") {
       RadarParams p { 300e9, 4e9, 100e-6, 50e6, 100, 1 };
       Target t1 { 50.0, 0.0, 1.0, 0.0, 0.0   };   // no vibration
       Target t2 { 50.0, 0.0, 1.0, 0.0, 500.0 };   // freq set, amp = 0
       std::vector<std::complex<float>> b1(p.num_samples), b2(p.num_samples);
       generate_chirp_if(p, t1, 0, b1);
       generate_chirp_if(p, t2, 0, b2);
       for (int i = 0; i < p.num_samples; ++i)
           REQUIRE(std::abs(b1[i] - b2[i]) < 1e-5f);
   }

Expected Output
----------------

.. code-block:: text

   $ cd build && ctest --output-on-failure -V
   ---------------------------------------------------------------
   PASSED  IF signal — range bin accuracy within 1 bin/50 m stationary target
   PASSED  IF signal — range bin accuracy within 1 bin/20 m stationary target
   PASSED  IF signal — range bin accuracy within 1 bin/80 m stationary target
   PASSED  IF signal — buffer size equals num_samples
   PASSED  IF signal — all samples have unit magnitude
   PASSED  Micro-Doppler — vibration increases slow-time phase variance
   PASSED  Micro-Doppler — zero amplitude equals static model
   ---------------------------------------------------------------
   7 tests passed  |  0 failed  |  0 skipped
