#include <catch2/catch_test_macros.hpp>
#include "fmcw_generator.hpp"

#include <cmath>
#include <complex>
#include <numeric>
#include <vector>

// ── Helper: compute variance of a float vector ────────────────────────────
static float variance(const std::vector<float>& v)
{
    if (v.empty()) return 0.0f;
    const float n    = static_cast<float>(v.size());
    const float mean = std::accumulate(v.begin(), v.end(), 0.0f) / n;
    float var = 0.0f;
    for (float x : v) var += (x - mean) * (x - mean);
    return var / n;
}

// ── Helper: collect phase DIFFERENCE between consecutive chirps ───────────
//
// Returns phase[c+1] - phase[c] for c in [0, num_chirps-2].
// Using differences removes the constant slow-time slope (velocity) and
// exposes the vibration-induced modulation — micro-Doppler sidebands.
// This avoids the phase-wrap issue that makes std::arg return 0 for
// stationary targets when measured at sample 0 (where t=0, phase=0 always).
static std::vector<float> slow_time_phase_diffs(
    const RadarParams& p,
    const Target&      tgt)
{
    std::vector<std::complex<float>> buf(p.num_samples);
    std::vector<float> phases;
    phases.reserve(p.num_chirps);

    // Sample at mid-chirp (t > 0) so phase is non-zero
    const int mid = p.num_samples / 2;

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

// ── Tests ─────────────────────────────────────────────────────────────────

TEST_CASE("Micro-Doppler — vibration increases slow-time phase variance",
          "[thz][microdoppler]")
{
    // 16 chirps gives enough slow-time samples to observe the vibration variance.
    const RadarParams p{ 300e9, 4e9, 100e-6, 50e6, 200, 16 };

    const Target no_vib  { 50.0, 0.0, 1.0, 0.0,    0.0   };
    const Target with_vib{ 50.0, 0.0, 1.0, 0.0002, 200.0 };

    const auto diffs_flat = slow_time_phase_diffs(p, no_vib);
    const auto diffs_vib  = slow_time_phase_diffs(p, with_vib);

    const float var_flat = variance(diffs_flat);
    const float var_vib  = variance(diffs_vib);

    INFO("var(no vibration)   = " << var_flat);
    INFO("var(with vibration) = " << var_vib);

    // A vibrating target must produce strictly greater inter-chirp phase variance
    REQUIRE(var_vib > var_flat);
}

TEST_CASE("Micro-Doppler — zero amplitude equals static model",
          "[thz][microdoppler]")
{
    // Setting vib_amp = 0 must produce output identical to no vibration,
    // regardless of what vib_freq is set to.
    const RadarParams p{ 300e9, 4e9, 100e-6, 50e6, 100, 1 };

    const Target t_no_vib  { 50.0, 0.0, 1.0, 0.0, 0.0   };  // no vibration
    const Target t_zero_amp{ 50.0, 0.0, 1.0, 0.0, 500.0 };  // freq set, amp = 0

    std::vector<std::complex<float>> b1(p.num_samples);
    std::vector<std::complex<float>> b2(p.num_samples);
    generate_chirp_if(p, t_no_vib,   0, b1);
    generate_chirp_if(p, t_zero_amp, 0, b2);

    for (int i = 0; i < p.num_samples; ++i) {
        INFO("sample " << i << ": b1=" << b1[i] << "  b2=" << b2[i]);
        REQUIRE(std::abs(b1[i] - b2[i]) < 1e-5f);
    }
}

TEST_CASE("Micro-Doppler — larger amplitude increases phase spread",
          "[thz][microdoppler]")
{
    // Doubling vib_amp should increase the slow-time inter-chirp phase variance.
    const RadarParams p{ 300e9, 4e9, 100e-6, 50e6, 200, 16 };

    const Target small_vib{ 50.0, 0.0, 1.0, 0.0001, 200.0 };  // 0.1 mm
    const Target large_vib{ 50.0, 0.0, 1.0, 0.0002, 200.0 };  // 0.2 mm

    const auto diffs_small = slow_time_phase_diffs(p, small_vib);
    const auto diffs_large = slow_time_phase_diffs(p, large_vib);

    INFO("var(0.1 mm) = " << variance(diffs_small));
    INFO("var(0.2 mm) = " << variance(diffs_large));

    REQUIRE(variance(diffs_large) > variance(diffs_small));
}
