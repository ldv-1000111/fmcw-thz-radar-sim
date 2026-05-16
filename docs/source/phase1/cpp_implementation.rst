.. _phase1_cpp_implementation:

C++ Implementation
===================

This page contains the complete source code for Phase 1.
Implement the files in the order presented.

``include/fmcw_generator.hpp``
--------------------------------

Defines the two POD structs and the ``generate_chirp_if()`` declaration.
Using POD structs enables pre-allocation and avoids heap churn in the
real-time loop.

.. code-block:: cpp
   :caption: include/fmcw_generator.hpp
   :linenos:

   #pragma once
   #include <complex>
   #include <vector>
   #include <cmath>

   // ── Radar hardware parameters ─────────────────────────────────
   struct RadarParams {
       double f0;           // Start frequency (Hz)  e.g. 300e9 for THz
       double bandwidth;    // Sweep bandwidth (Hz)  e.g. 4e9 → 3.75 cm res
       double chirp_time;   // Chirp duration (s)    e.g. 100e-6
       double fs;           // IF sampling rate (Hz) e.g. 50e6
       int    num_samples;  // Samples per chirp
       int    num_chirps;   // Chirps per frame
   };

   // ── Target model (includes THz micro-Doppler) ─────────────────
   struct Target {
       double range;      // Initial range (m)
       double velocity;   // Radial velocity (m/s)
       double rcs;        // Radar cross-section (m²)
       double vib_amp;    // Vibration amplitude (m)  — THz micro-Doppler
       double vib_freq;   // Vibration frequency (Hz) — set 0.0 to disable
   };

   // Pre-allocated IF signal for one chirp — caller owns the buffer
   void generate_chirp_if(
       const RadarParams& p,
       const Target&      tgt,
       int                chirp_idx,
       std::vector<std::complex<float>>& out   // size must equal p.num_samples
   );

``include/csv_export.hpp``
----------------------------

Lightweight header-only helper for writing IF signal samples to CSV.
Used by ``main.cpp`` and the Python validation script.

.. code-block:: cpp
   :caption: include/csv_export.hpp
   :linenos:

   #pragma once
   #include <complex>
   #include <fstream>
   #include <stdexcept>
   #include <string>
   #include <vector>

   inline void write_if_csv(
       const std::string& path,
       const std::vector<std::complex<float>>& sig)
   {
       std::ofstream f(path);
       if (!f) throw std::runtime_error("Cannot open: " + path);
       f << "sample,real,imag\n";
       for (std::size_t i = 0; i < sig.size(); ++i)
           f << i << ',' << sig[i].real() << ',' << sig[i].imag() << '\n';
   }

``src/fmcw_generator.cpp``
----------------------------

.. important::

   The ``out`` buffer is **pre-allocated by the caller**. Never call ``new``,
   ``malloc``, or ``push_back`` inside the per-sample loop. Allocate all
   buffers once at initialisation and reuse across frames.

.. code-block:: cpp
   :caption: src/fmcw_generator.cpp
   :linenos:

   #include "fmcw_generator.hpp"

   static constexpr double C = 299792458.0;  // speed of light (m/s)

   void generate_chirp_if(
       const RadarParams& p,
       const Target&      tgt,
       int                chirp_idx,
       std::vector<std::complex<float>>& out)
   {
       const double slow_t = chirp_idx * p.chirp_time;
       const double slope  = p.bandwidth / p.chirp_time;

       for (int i = 0; i < p.num_samples; ++i) {
           const double t = static_cast<double>(i) / p.fs;  // fast-time

           // Target displacement — includes THz micro-Doppler vibration
           double R = tgt.range
                    + tgt.velocity * (slow_t + t)
                    + tgt.vib_amp  * std::sin(2.0*M_PI*tgt.vib_freq*(slow_t+t));

           const double tau       = 2.0 * R / C;
           const double beat_freq = slope * tau
                                  + 2.0 * p.f0 * tgt.velocity / C;
           const double phase     = 2.0 * M_PI * beat_freq * t;

           out[i] = std::complex<float>(
               static_cast<float>(std::cos(phase)),
               static_cast<float>(std::sin(phase)));
       }
   }

``src/main.cpp``
-----------------

CLI entry point: configures a 300 GHz THz radar, generates one chirp
for a target at 50 m with 200 Hz engine vibration, and writes
``if_signal.csv`` for external validation.

.. code-block:: cpp
   :caption: src/main.cpp
   :linenos:

   #include "fmcw_generator.hpp"
   #include "csv_export.hpp"
   #include <cstdio>

   int main() {
       // ── 300 GHz THz radar parameters ──────────────────────────
       const RadarParams p {
           300e9,    // f0 (Hz)           — 300 GHz carrier
           4e9,      // bandwidth (Hz)    — 3.75 cm range resolution
           100e-6,   // chirp_time (s)
           50e6,     // fs (Hz)
           5000,     // num_samples
           256       // num_chirps (used in Phase 2)
       };

       // ── Target: 50 m, stationary, 200 Hz engine micro-Doppler ─
       const Target tgt {
           50.0,     // range (m)
           0.0,      // velocity (m/s)
           1.0,      // rcs (m²)
           0.0002,   // vib_amp (m) = 0.2 mm
           200.0     // vib_freq (Hz)
       };

       std::vector<std::complex<float>> if_sig(p.num_samples);

       generate_chirp_if(p, tgt, 0, if_sig);
       write_if_csv("if_signal.csv", if_sig);

       std::printf("Phase 1: wrote %zu samples to if_signal.csv\n",
                   if_sig.size());
       return 0;
   }
