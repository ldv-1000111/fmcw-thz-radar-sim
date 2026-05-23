#include "signal_processing.hpp"

#include <cmath>
#include <complex>
#include <fftw3.h>

// Pre-compute a Hann window of length N
static std::vector<float> hann_window(int N)
{
    std::vector<float> w(N);
    for (int i = 0; i < N; ++i)
        w[i] = 0.5f * (1.0f - std::cos(2.0f * static_cast<float>(M_PI) * i / (N - 1)));
    return w;
}

void compute_range_doppler(
    const std::vector<std::vector<std::complex<float>>>& cube,
    int num_chirps,
    int num_samples,
    std::vector<std::vector<float>>& rd_map)
{
    // Pre-compute Hann window for range FFT sidelobe suppression.
    // Without windowing the strong 20m stationary target leaks into
    // adjacent range bins at Doppler bin 0, masking weaker targets.
    const auto win = hann_window(num_samples);

    // ── Step 1: Range FFT along fast-time axis ────────────────────────────
    std::vector<fftwf_complex> rbuf(num_samples);

    // Use FFTW_MEASURE in production for best runtime performance;
    // FFTW_ESTIMATE skips the planning phase (faster first run, slower FFT)
    fftwf_plan rplan = fftwf_plan_dft_1d(
        num_samples, rbuf.data(), rbuf.data(),
        FFTW_FORWARD, FFTW_ESTIMATE);

    std::vector<std::vector<std::complex<float>>> rout(
        num_chirps, std::vector<std::complex<float>>(num_samples));

    for (int c = 0; c < num_chirps; ++c) {
        for (int s = 0; s < num_samples; ++s) {
            rbuf[s][0] = cube[c][s].real() * win[s];
            rbuf[s][1] = cube[c][s].imag() * win[s];
        }
        fftwf_execute(rplan);
        for (int s = 0; s < num_samples; ++s)
            rout[c][s] = {rbuf[s][0], rbuf[s][1]};
    }
    fftwf_destroy_plan(rplan);

    // ── Step 2: Doppler FFT along slow-time axis ──────────────────────────
    std::vector<fftwf_complex> dbuf(num_chirps);
    fftwf_plan dplan = fftwf_plan_dft_1d(
        num_chirps, dbuf.data(), dbuf.data(),
        FFTW_FORWARD, FFTW_ESTIMATE);

    rd_map.assign(num_chirps, std::vector<float>(num_samples, 0.0f));

    for (int s = 0; s < num_samples; ++s) {
        for (int c = 0; c < num_chirps; ++c) {
            dbuf[c][0] = rout[c][s].real();
            dbuf[c][1] = rout[c][s].imag();
        }
        fftwf_execute(dplan);
        for (int c = 0; c < num_chirps; ++c)
            rd_map[c][s] = std::hypot(dbuf[c][0], dbuf[c][1]);
    }
    fftwf_destroy_plan(dplan);
}
