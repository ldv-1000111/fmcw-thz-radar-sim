#pragma once
#include <complex>
#include <vector>
#include <cmath>

// ── Radar hardware parameters ─────────────────────────────────────────────
struct RadarParams {
    double f0;           // Start frequency (Hz)   e.g. 300e9 for 300 GHz THz
    double bandwidth;    // Sweep bandwidth (Hz)   e.g. 4e9  → 3.75 cm range res
    double chirp_time;   // Chirp duration (s)     e.g. 100e-6
    double fs;           // IF sampling rate (Hz)  e.g. 50e6
    int    num_samples;  // Samples per chirp      e.g. 5000
    int    num_chirps;   // Chirps per frame        e.g. 256 (used in Phase 2)
};

// ── Target model — includes THz micro-Doppler vibration ──────────────────
struct Target {
    double range;      // Initial range (m)
    double velocity;   // Radial velocity (m/s)   positive = approaching
    double rcs;        // Radar cross-section (m²)
    double vib_amp;    // Vibration amplitude (m)  e.g. 0.0002 = 0.2 mm
    double vib_freq;   // Vibration frequency (Hz) e.g. 200.0 (engine idle)
                       // Set vib_amp = 0.0 to disable micro-Doppler
};

// ── IF signal generator ───────────────────────────────────────────────────
//
// Generates the complex analytic IF beat signal for one chirp of an FMCW
// radar observing a single target. The output buffer must be pre-allocated
// by the caller to exactly p.num_samples elements.
//
// Physics:
//   beat_freq = (B/Tc) * tau + (2*f0*v / c)
//   tau(t)    = 2 * (R + v*t + A*sin(2*pi*fv*t)) / c
//   s_IF[i]   = exp(j * 2*pi * beat_freq * t)   where t = i / fs
//
// Never call this with out.size() != p.num_samples.
void generate_chirp_if(
    const RadarParams&                p,
    const Target&                     tgt,
    int                               chirp_idx,  // slow-time index [0, num_chirps)
    std::vector<std::complex<float>>& out          // PRE-ALLOCATED, size = num_samples
);
