#include "fmcw_generator.hpp"
#include "signal_processing.hpp"
#include "csv_export.hpp"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <vector>
#include <complex>

int main(int argc, char* argv[])
{
    (void)argc;

    // ── Radar parameters ──────────────────────────────────────────────────
    const RadarParams p {
        300e9,    // f0          (Hz)  lambda = 1 mm
        4e9,      // bandwidth   (Hz)  delta_r = 3.75 cm
        100e-6,   // chirp_time  (s)
        50e6,     // fs          (Hz)
        256,      // num_samples
        256       // num_chirps  — 256 chirps for good Doppler resolution
    };

    // ── Three targets ─────────────────────────────────────────────────────
    // bin_res = c / (2 * f0 * NC * TC) = 0.01953 m/s/bin  (NC=256)
    //   target 1: v= 0.000 m/s → Doppler bin   0
    //   target 2: v=+0.625 m/s → Doppler bin  32
    //   target 3: v=+4.375 m/s → Doppler bin 224
    const std::vector<Target> targets = {
        { 20.0,  0.000, 1.0, 0.0002, 200.0 },  // 20 m, stationary
        { 50.0,  0.625, 1.0, 0.0,    0.0   },  // 50 m, v=+0.625 m/s → bin 32
        { 80.0,  4.375, 1.0, 0.0,    0.0   },  // 80 m, v=+4.375 m/s → bin 224
    };

    // ── Output path: write next to the binary ─────────────────────────────
    const std::filesystem::path bin_dir =
        std::filesystem::path(argv[0]).parent_path();

    // ── Phase 1: generate single-chirp IF signal for range validation ─────
    {
        std::vector<std::complex<float>> if_sig(p.num_samples);
        generate_chirp_if(p, targets[1], 0, if_sig);   // 50 m target

        const auto out = bin_dir / "if_signal.csv";
        write_if_csv(out.string(), if_sig);
        std::printf("Phase 1: wrote %zu samples -> %s\n",
                    if_sig.size(), out.string().c_str());
    }

    // ── Phase 2: build data cube, compute Range-Doppler map ───────────────
    {
        // Superpose all targets in each chirp
        std::vector<std::vector<std::complex<float>>> cube(
            p.num_chirps,
            std::vector<std::complex<float>>(p.num_samples,
                                             std::complex<float>(0.0f, 0.0f)));

        // Superpose all targets in each chirp.
        // Moving targets are scaled by amplitude_scale to ensure they are
        // visible above the range FFT sidelobes of the stationary target.
        const std::vector<float> amp_scale = { 1.0f, 10.0f, 10.0f };

        std::vector<std::complex<float>> buf(p.num_samples);
        for (int t = 0; t < static_cast<int>(targets.size()); ++t) {
            for (int c = 0; c < p.num_chirps; ++c) {
                generate_chirp_if(p, targets[t], c, buf);
                for (int s = 0; s < p.num_samples; ++s)
                    cube[c][s] += buf[s] * amp_scale[t];
            }
        }

        // Compute Range-Doppler map
        std::vector<std::vector<float>> rd_map;
        compute_range_doppler(cube, p.num_chirps, p.num_samples, rd_map);

        // Write range_doppler.csv (doppler_bin, range_bin, magnitude)
        const auto out = bin_dir / "range_doppler.csv";
        std::ofstream f(out);
        f << "doppler_bin,range_bin,magnitude\n";
        f.precision(6);
        f << std::fixed;
        for (int d = 0; d < p.num_chirps; ++d)
            for (int r = 0; r < p.num_samples; ++r)
                f << d << ',' << r << ',' << rd_map[d][r] << '\n';

        std::printf("Phase 2: wrote %dx%d Range-Doppler map -> %s\n",
                    p.num_chirps, p.num_samples, out.string().c_str());
        std::printf("         targets: 20m/0m/s(bin0)  50m/+0.625m/s(bin32)  80m/+4.375m/s(bin224)\n");
    }

    return 0;
}
