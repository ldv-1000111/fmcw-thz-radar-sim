#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "fmcw_generator.hpp"
#include "signal_processing.hpp"

#include <algorithm>
#include <complex>
#include <cmath>
#include <vector>

static constexpr double C    = 299792458.0;
static constexpr double F0   = 300e9;
static constexpr double BW   = 4e9;
static constexpr double TC   = 100e-6;
static constexpr double FS   = 50e6;
static constexpr int    NS   = 256;   // num_samples (small for fast tests)
static constexpr int    NC   = 256;    // num_chirps

// Expected range bin for a stationary target
static int expected_range_bin(double range)
{
    const double tau    = 2.0 * range / C;
    const double f_beat = (BW / TC) * tau;
    return static_cast<int>(std::round(f_beat * NS / FS));
}

// Build a full data cube for a single stationary target
static std::vector<std::vector<std::complex<float>>>
make_cube(double range, double velocity = 0.0,
          double vib_amp = 0.0, double vib_freq = 0.0)
{
    const RadarParams p{ F0, BW, TC, FS, NS, NC };
    const Target      tgt{ range, velocity, 1.0, vib_amp, vib_freq };

    std::vector<std::vector<std::complex<float>>> cube(
        NC, std::vector<std::complex<float>>(NS));

    for (int c = 0; c < NC; ++c)
        generate_chirp_if(p, tgt, c, cube[c]);

    return cube;
}

// ── Tests ─────────────────────────────────────────────────────────────────

TEST_CASE("Range-Doppler — output dimensions match input", "[rd][sanity]")
{
    auto cube = make_cube(50.0);
    std::vector<std::vector<float>> rd_map;
    compute_range_doppler(cube, NC, NS, rd_map);

    REQUIRE(static_cast<int>(rd_map.size())    == NC);
    REQUIRE(static_cast<int>(rd_map[0].size()) == NS);
}

TEST_CASE("Range-Doppler — stationary target peak at correct range bin",
          "[rd][range]")
{
    // Target at 30 m — find the range bin with maximum energy
    const double target_range = 30.0;
    auto cube = make_cube(target_range);
    std::vector<std::vector<float>> rd_map;
    compute_range_doppler(cube, NC, NS, rd_map);

    // Sum over all Doppler bins to get range profile
    std::vector<float> range_profile(NS, 0.0f);
    for (int d = 0; d < NC; ++d)
        for (int r = 0; r < NS; ++r)
            range_profile[r] += rd_map[d][r];

    const int peak_r = static_cast<int>(
        std::max_element(range_profile.begin(), range_profile.end())
        - range_profile.begin());

    const int expected = expected_range_bin(target_range);
    INFO("peak range bin=" << peak_r << " expected=" << expected);
    REQUIRE(std::abs(peak_r - expected) <= 2);
}

TEST_CASE("Range-Doppler — all output values are non-negative", "[rd][sanity]")
{
    auto cube = make_cube(40.0);
    std::vector<std::vector<float>> rd_map;
    compute_range_doppler(cube, NC, NS, rd_map);

    for (int d = 0; d < NC; ++d)
        for (int r = 0; r < NS; ++r) {
            INFO("rd_map[" << d << "][" << r << "] = " << rd_map[d][r]);
            REQUIRE(rd_map[d][r] >= 0.0f);
        }
}

TEST_CASE("Range-Doppler — moving target has higher off-DC Doppler energy",
          "[rd][doppler]")
{
    // A moving target spreads energy to non-zero Doppler bins.
    // The sum of energy in bins [1..NC/2] must be greater for a moving
    // target than for a stationary one at the same range.
    const double bin_res = 3e8 / (2.0 * F0 * NC * TC);
    const double v_moving = 8.0 * bin_res;   // bin 8

    auto cube_static = make_cube(50.0, 0.0);
    auto cube_moving = make_cube(50.0, v_moving);

    std::vector<std::vector<float>> rd_static, rd_moving;
    compute_range_doppler(cube_static, NC, NS, rd_static);
    compute_range_doppler(cube_moving, NC, NS, rd_moving);

    const int r_bin = expected_range_bin(50.0);

    // Sum energy in off-DC Doppler bins at the target's range bin
    float energy_static = 0.0f, energy_moving = 0.0f;
    for (int d = 1; d < NC / 2; ++d) {
        energy_static += rd_static[d][r_bin];
        energy_moving += rd_moving[d][r_bin];
    }

    INFO("off-DC energy: static=" << energy_static
         << "  moving=" << energy_moving);

    // Moving target must have more off-DC Doppler energy
    REQUIRE(energy_moving > energy_static);
}
