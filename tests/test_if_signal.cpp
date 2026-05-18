#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "fmcw_generator.hpp"

#include <algorithm>
#include <cmath>
#include <complex>
#include <vector>

static constexpr double C = 299792458.0;

// ── Helpers ───────────────────────────────────────────────────────────────

// Expected beat-frequency bin for a stationary target (velocity = 0).
// k = round( f_beat * N_s / f_s )   where  f_beat = (B/Tc) * (2R/c)
static int expected_range_bin(const RadarParams& p, double range)
{
    const double tau    = 2.0 * range / C;
    const double f_beat = (p.bandwidth / p.chirp_time) * tau;
    return static_cast<int>(std::round(f_beat * p.num_samples / p.fs));
}

// Brute-force DFT magnitude peak — O(N²), acceptable for small N in tests.
// Returns the bin index [0, N/2) with the largest magnitude.
static int peak_bin(const std::vector<std::complex<float>>& sig)
{
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
        if (mag > best_mag) {
            best_mag = mag;
            best_k   = k;
        }
    }
    return best_k;
}

// ── Test suite ────────────────────────────────────────────────────────────

// Use a small num_samples (500) so the brute-force DFT is fast in CI.
// The 300 GHz parameters match main.cpp exactly.
static RadarParams make_params()
{
    return RadarParams{ 300e9, 4e9, 100e-6, 50e6, 500, 1 };
}

// ── Range accuracy ────────────────────────────────────────────────────────

TEST_CASE("IF signal — range bin accuracy within 1 bin", "[fmcw][range]")
{
    const RadarParams p = make_params();

    SECTION("50 m stationary target")
    {
        const Target tgt{ 50.0, 0.0, 1.0, 0.0, 0.0 };
        std::vector<std::complex<float>> buf(p.num_samples);
        generate_chirp_if(p, tgt, 0, buf);

        const int got      = peak_bin(buf);
        const int expected = expected_range_bin(p, 50.0);
        INFO("got=" << got << " expected=" << expected);
        REQUIRE(std::abs(got - expected) <= 1);
    }

    SECTION("20 m stationary target")
    {
        const Target tgt{ 20.0, 0.0, 1.0, 0.0, 0.0 };
        std::vector<std::complex<float>> buf(p.num_samples);
        generate_chirp_if(p, tgt, 0, buf);

        const int got      = peak_bin(buf);
        const int expected = expected_range_bin(p, 20.0);
        INFO("got=" << got << " expected=" << expected);
        REQUIRE(std::abs(got - expected) <= 1);
    }

    SECTION("80 m stationary target")
    {
        const Target tgt{ 80.0, 0.0, 1.0, 0.0, 0.0 };
        std::vector<std::complex<float>> buf(p.num_samples);
        generate_chirp_if(p, tgt, 0, buf);

        const int got      = peak_bin(buf);
        const int expected = expected_range_bin(p, 80.0);
        INFO("got=" << got << " expected=" << expected);
        REQUIRE(std::abs(got - expected) <= 1);
    }
}

// ── Sanity checks ─────────────────────────────────────────────────────────

TEST_CASE("IF signal — output buffer size equals num_samples", "[fmcw][sanity]")
{
    const RadarParams p   = make_params();
    const Target      tgt{ 30.0, 5.0, 1.0, 0.0, 0.0 };

    std::vector<std::complex<float>> buf(p.num_samples);
    generate_chirp_if(p, tgt, 0, buf);

    REQUIRE(static_cast<int>(buf.size()) == p.num_samples);
}

TEST_CASE("IF signal — all samples have unit magnitude", "[fmcw][sanity]")
{
    // Unit magnitude is expected because generate_chirp_if produces
    // exp(j*phase) = cos(phase) + j*sin(phase), ignoring RCS amplitude.
    const RadarParams p   = make_params();
    const Target      tgt{ 50.0, 0.0, 1.0, 0.0, 0.0 };

    std::vector<std::complex<float>> buf(p.num_samples);
    generate_chirp_if(p, tgt, 0, buf);

    for (int i = 0; i < p.num_samples; ++i) {
        const float mag = std::abs(buf[i]);
        INFO("sample " << i << ": |s| = " << mag);
        REQUIRE(std::abs(mag - 1.0f) < 1e-4f);
    }
}

TEST_CASE("IF signal — chirp index shifts slow-time phase", "[fmcw][chirp]")
{
    // Two consecutive chirps of a moving target must produce different phases
    // at a non-zero sample index (sample 0 is always phase=0 because t=0).
    const RadarParams p   = make_params();
    const Target      tgt{ 50.0, 5.0, 1.0, 0.0, 0.0 };  // v = 5 m/s

    std::vector<std::complex<float>> chirp0(p.num_samples);
    std::vector<std::complex<float>> chirp1(p.num_samples);
    generate_chirp_if(p, tgt, 0, chirp0);
    generate_chirp_if(p, tgt, 1, chirp1);

    // Compare at sample N/2 where the beat phase is well away from 0
    const int mid = p.num_samples / 2;
    const float phase0 = std::arg(chirp0[mid]);
    const float phase1 = std::arg(chirp1[mid]);
    INFO("phase at sample " << mid << ": chirp0=" << phase0 << "  chirp1=" << phase1);
    REQUIRE(std::abs(phase0 - phase1) > 1e-3f);
}
