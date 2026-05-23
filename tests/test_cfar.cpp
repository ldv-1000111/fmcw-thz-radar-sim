#include <catch2/catch_test_macros.hpp>
#include "cfar.hpp"

#include <cmath>
#include <vector>

// ── Helpers ───────────────────────────────────────────────────────────────

// Build a flat noise floor with a single spike at position spike_pos
static std::vector<float> make_row(int N, int spike_pos, float noise_level,
                                   float spike_height)
{
    std::vector<float> row(N, noise_level);
    row[spike_pos] = spike_height;
    return row;
}

// ── Tests ─────────────────────────────────────────────────────────────────

TEST_CASE("CA-CFAR — detects a strong spike above noise floor", "[cfar]")
{
    // 100-element row, flat noise at 1.0, spike at position 50 = 10.0
    // With guard=2, train=8, alpha=2.0: threshold = 2.0 * 1.0 = 2.0
    // spike (10.0) > threshold (2.0) → should detect
    const auto row = make_row(100, 50, 1.0f, 10.0f);
    REQUIRE(cfar_detect(row, 50, 2, 8, 2.0f) == true);
}

TEST_CASE("CA-CFAR — does not flag a cell at noise floor level", "[cfar]")
{
    // Uniform noise, no spike — every cell equals neighbours
    // threshold = alpha * mean_noise = alpha * noise_level
    // cell value = noise_level, so cell <= threshold → no detection
    const int   N   = 100;
    const float lvl = 1.0f;
    std::vector<float> row(N, lvl);

    // With alpha=2.0 the threshold is 2.0 * 1.0 = 2.0
    // No cell exceeds 1.0 so none should be detected
    for (int i = 10; i < N - 10; ++i)
        REQUIRE(cfar_detect(row, i, 2, 8, 2.0f) == false);
}

TEST_CASE("CA-CFAR — guard cells are excluded from noise estimate", "[cfar]")
{
    // Place two spikes at guard cells (±1 from CUT at position 50)
    // Guard cells must be excluded; if they were included the noise
    // estimate would be inflated and the CUT might not be detected.
    std::vector<float> row(100, 0.5f);
    row[49] = 20.0f;   // guard cell — must be excluded
    row[50] = 5.0f;    // CUT
    row[51] = 20.0f;   // guard cell — must be excluded

    // With guard=1, the CUT neighbour spikes are excluded from noise.
    // Noise estimate ≈ 0.5, threshold = 2.0 * 0.5 = 1.0
    // CUT (5.0) > threshold (1.0) → should detect
    REQUIRE(cfar_detect(row, 50, 1, 8, 2.0f) == true);
}

TEST_CASE("CA-CFAR — near edge of row does not crash", "[cfar][edge]")
{
    // CUT at position 2, close to left edge — guard+train extends beyond 0
    // cfar_detect must clamp lo=max(0,...) without undefined behaviour
    const auto row = make_row(50, 2, 1.0f, 8.0f);
    REQUIRE_NOTHROW(cfar_detect(row, 2, 2, 5, 2.0f));
}

TEST_CASE("CA-CFAR — alpha scales the detection threshold", "[cfar]")
{
    // spike at 50, noise = 1.0, spike = 3.0
    // alpha=1.5 → threshold=1.5 → 3.0 > 1.5 → detect
    // alpha=4.0 → threshold=4.0 → 3.0 < 4.0 → no detect
    const auto row = make_row(100, 50, 1.0f, 3.0f);
    REQUIRE(cfar_detect(row, 50, 2, 8, 1.5f) == true);
    REQUIRE(cfar_detect(row, 50, 2, 8, 4.0f) == false);
}
