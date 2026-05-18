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

        // ── Target range at (slow_t + t) — includes micro-Doppler ────────
        //
        //   R(t) = R0 + v*(slow_t + t) + A_vib * sin(2*pi*f_vib*(slow_t+t))
        //
        // The vibration term models THz micro-Doppler:
        //   - Engine idle:      A_vib ≈ 0.2 mm, f_vib ≈ 200 Hz
        //   - Drone propeller:  A_vib ≈ 0.5 mm, f_vib ≈ 80–120 Hz
        //   - Set vib_amp = 0 to get a clean stationary/moving target
        const double total_t = slow_t + t;
        const double R = tgt.range
                       + tgt.velocity * total_t
                       + tgt.vib_amp  * std::sin(2.0 * M_PI * tgt.vib_freq * total_t);

        // ── Round-trip delay (seconds) ────────────────────────────────────
        const double tau = 2.0 * R / C;

        // ── IF beat frequency (Hz) ─────────────────────────────────────────
        //
        //   f_beat = (B/Tc) * tau   +   2*f0*v / c
        //            ^^^range^^^         ^^^Doppler^^^
        const double beat_freq = slope * tau
                               + 2.0 * p.f0 * tgt.velocity / C;

        // ── Complex IF sample: exp(j * 2*pi * f_beat * t) ────────────────
        const double phase = 2.0 * M_PI * beat_freq * t;

        out[i] = std::complex<float>(
            static_cast<float>(std::cos(phase)),
            static_cast<float>(std::sin(phase)));
    }
}
