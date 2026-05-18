#pragma once
#include <complex>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

// ── CSV export helper ─────────────────────────────────────────────────────
//
// Writes a single chirp IF signal to a CSV file with three columns:
//   sample,real,imag
//
// Used by main.cpp to generate if_signal.csv for Python validation.
// Header-only — no separate .cpp needed.
inline void write_if_csv(
    const std::string&                       path,
    const std::vector<std::complex<float>>& sig)
{
    std::ofstream f(path);
    if (!f) throw std::runtime_error("write_if_csv: cannot open file: " + path);

    f << "sample,real,imag\n";
    f.precision(8);
    f << std::fixed;

    for (std::size_t i = 0; i < sig.size(); ++i) {
        f << i          << ','
          << sig[i].real() << ','
          << sig[i].imag() << '\n';
    }
}
