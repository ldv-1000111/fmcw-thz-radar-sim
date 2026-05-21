#pragma once
#include <complex>
#include <vector>

// Compute 2D Range-Doppler magnitude map from data cube.
// cube[chirp][sample] → rd_map[doppler_bin][range_bin]
void compute_range_doppler(
    const std::vector<std::vector<std::complex<float>>>& cube,
    int num_chirps,
    int num_samples,
    std::vector<std::vector<float>>& rd_map   // output: magnitude
);
