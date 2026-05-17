#include "fmcw_generator.hpp"
#include "csv_export.hpp"
#include <cstdio>
#include <vector>
#include <complex>

int main()
{
    // 300 GHz THz radar:
    //   f0=300GHz -> lambda=1mm
    //   bandwidth=4GHz -> delta_r=c/(2B)=3.75cm
    const RadarParams p {
        300e9,    // f0          (Hz)
        4e9,      // bandwidth   (Hz)
        100e-6,   // chirp_time  (s)
        50e6,     // fs          (Hz)
        5000,     // num_samples
        256       // num_chirps  (used in Phase 2)
    };

    // Target: 50 m, stationary, 200 Hz engine micro-Doppler
    //   expected range bin = round((B/Tc)*(2R/c)*Ns/fs) = 1334
    const Target tgt {
        50.0,     // range    (m)
        0.0,      // velocity (m/s)
        1.0,      // rcs      (m^2)
        0.0002,   // vib_amp  (m) = 0.2 mm
        200.0     // vib_freq (Hz)
    };

    std::vector<std::complex<float>> if_sig(p.num_samples);
    generate_chirp_if(p, tgt, 0, if_sig);

    const char* out_path = "if_signal.csv";
    write_if_csv(out_path, if_sig);

    std::printf("Phase 1: wrote %zu samples -> %s\n",
                if_sig.size(), out_path);
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