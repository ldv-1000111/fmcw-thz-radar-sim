#include "fmcw_generator.hpp"

#include <cassert>
#include <cmath>

// Speed of light (m/s)
static constexpr double C = 299792458.0;

void generate_chirp_if(
    const RadarParams&                p,
    const Target&                     tgt,
    int                               chirp_idx,
    std::vector<std::complex<float>>& out)
{
    assert(static_cast<int>(out.size()) == p.num_samples &&
           "generate_chirp_if: out buffer size must equal p.num_samples");

    // Slow-time offset for this chirp (seconds from start of frame)
    const double slow_t = static_cast<double>(chirp_idx) * p.chirp_time;

    // Chirp slope (Hz/s)
    const double slope = p.bandwidth / p.chirp_time;

    for (int i = 0; i < p.num_samples; ++i) {
        // Fast-time sample instant (seconds within the chirp)
        const double t = static_cast<double>(i) / p.fs;

        // ── Range at start of this chirp (slow-time only) ────────────────
        //
        // In the FMCW deramping model the IF signal carries two phase terms:
        //
        //   phi_IF(t) = 2*pi*f0*tau0  +  2*pi*(B/Tc)*tau0*t
        //               ^^^^^^^^^^^^     ^^^^^^^^^^^^^^^^^^^^
        //               initial phase    beat tone (range FFT bin)
        //               (Doppler)
        //
        // The initial phase phi0 = 2*pi*f0*tau0 advances by
        //   2*pi * 2*f0*v/c * Tc
        // between consecutive chirps — this is the Doppler phase that the
        // slow-time (Doppler) FFT must see.  Without phi0, the range FFT
        // output has no inter-chirp phase change and the Doppler FFT
        // produces no peak for moving targets.
        // ── Instantaneous range at chirp start + vibration phase ─────────
        // Vibration modulates the initial phase (micro-Doppler sidebands):
        //   R0(slow_t) = R_base + v*slow_t + A_vib*sin(2*pi*f_vib*slow_t)
        const double R0   = tgt.range
                          + tgt.velocity * slow_t
                          + tgt.vib_amp  * std::sin(2.0 * M_PI * tgt.vib_freq * slow_t);
        const double tau0 = 2.0 * R0 / C;
        const double phi0 = 2.0 * M_PI * p.f0 * tau0;   // Doppler carrier

        // ── IF beat frequency ─────────────────────────────────────────────
        //   f_beat = (B/Tc)*tau0  +  2*f0*v/c   (use tau0 for range bin)
        const double beat_freq = slope * tau0
                               + 2.0 * p.f0 * tgt.velocity / C;

        // ── Full IF phase: initial phase + beat tone ─────────────────────
        //   phi(t) = phi0 + 2*pi*f_beat*t
        // (tau vs tau0: vibration modulates phi0 for micro-Doppler sidebands)
        const double phase = phi0 + 2.0 * M_PI * beat_freq * t;

        out[i] = std::complex<float>(
            static_cast<float>(std::cos(phase)),
            static_cast<float>(std::sin(phase)));
    }
}
