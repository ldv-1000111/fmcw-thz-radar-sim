#include "fmcw_generator.hpp"
#include "csv_export.hpp"

#include <cstdio>
#include <filesystem>
#include <vector>
#include <complex>

int main(int argc, char* argv[])
{
    (void)argc;  // unused — only argv[0] is needed for the output path
    // ── 300 GHz THz radar parameters ──────────────────────────────────────
    //
    //   f0        = 300 GHz  →  lambda = 1 mm
    //   bandwidth =   4 GHz  →  range resolution = c/(2B) = 3.75 cm
    //   chirp_time = 100 us
    //   fs         =  50 MHz →  num_samples = fs * chirp_time = 5000
    //   num_chirps =   256   →  used in Phase 2 Range-Doppler
    //
    const RadarParams p {
        300e9,    // f0          (Hz)
        4e9,      // bandwidth   (Hz)
        100e-6,   // chirp_time  (s)
        50e6,     // fs          (Hz)
        5000,     // num_samples
        256       // num_chirps
    };

    // ── Target: 50 m, stationary, 200 Hz engine micro-Doppler ─────────────
    //
    //   range    = 50 m    →  expected range bin = 1334
    //   velocity = 0 m/s   →  no Doppler shift
    //   rcs      = 1.0 m^2 →  unit reflectivity
    //   vib_amp  = 0.2 mm  →  micro-Doppler amplitude (THz-visible)
    //   vib_freq = 200 Hz  →  engine idle vibration
    //
    const Target tgt {
        50.0,     // range    (m)
        0.0,      // velocity (m/s)
        1.0,      // rcs      (m^2)
        0.0002,   // vib_amp  (m) = 0.2 mm
        200.0     // vib_freq (Hz)
    };

    // ── Generate one chirp (chirp index 0) ────────────────────────────────
    std::vector<std::complex<float>> if_sig(p.num_samples);
    generate_chirp_if(p, tgt, 0, if_sig);

    // ── Output path: always write next to the binary ──────────────────────
    //
    // parent_path() of argv[0] gives the directory containing radar_sim,
    // regardless of where the user launches it from:
    //   ./build/radar_sim  →  writes  build/if_signal.csv
    //   ./radar_sim        →  writes  ./if_signal.csv
    //
    const std::filesystem::path out_path =
        std::filesystem::path(argv[0]).parent_path() / "if_signal.csv";

    write_if_csv(out_path.string(), if_sig);

    std::printf("Phase 1: wrote %zu samples -> %s\n",
                if_sig.size(), out_path.string().c_str());
    std::printf("         f0=%.0f GHz  B=%.0f GHz  "
                "Tc=%.0f us  fs=%.0f MHz\n",
                p.f0/1e9, p.bandwidth/1e9,
                p.chirp_time*1e6, p.fs/1e6);
    std::printf("         target: R=%.1f m  v=%.1f m/s  "
                "vib=%.3f mm @ %.0f Hz\n",
                tgt.range, tgt.velocity,
                tgt.vib_amp * 1000.0, tgt.vib_freq);

    return 0;
}
