.. _phase1_cpp_implementation:

C++ Implementation
===================

This page contains the **exact source code** committed to the repository.
All files listed here were verified to build and pass the full test suite
(7/7 Catch2 assertions, Python ``PASS``) before tagging ``v0.1.0``.

.. note::

   Two bugs were caught and fixed during the initial build verification
   and are documented in the :ref:`changelog`. The code on this page
   reflects the corrected, working versions.

----

``include/fmcw_generator.hpp``
--------------------------------

Defines the two POD structs and the ``generate_chirp_if()`` declaration.
Detailed physics comments are included so the header serves as
living documentation of the signal model.

.. code-block:: cpp
   :caption: include/fmcw_generator.hpp
   :linenos:

   #pragma once
   #include <complex>
   #include <vector>
   #include <cmath>

   // -- Radar hardware parameters
   struct RadarParams {
       double f0;           // Start frequency (Hz)   e.g. 300e9 for 300 GHz THz
       double bandwidth;    // Sweep bandwidth (Hz)   e.g. 4e9  -> 3.75 cm range res
       double chirp_time;   // Chirp duration (s)     e.g. 100e-6
       double fs;           // IF sampling rate (Hz)  e.g. 50e6
       int    num_samples;  // Samples per chirp      e.g. 5000
       int    num_chirps;   // Chirps per frame       e.g. 256 (used in Phase 2)
   };

   // -- Target model: includes THz micro-Doppler vibration
   struct Target {
       double range;      // Initial range (m)
       double velocity;   // Radial velocity (m/s)   positive = approaching
       double rcs;        // Radar cross-section (m^2)
       double vib_amp;    // Vibration amplitude (m)  e.g. 0.0002 = 0.2 mm
       double vib_freq;   // Vibration frequency (Hz) e.g. 200.0 (engine idle)
                          // Set vib_amp = 0.0 to disable micro-Doppler
   };

   // -- IF signal generator
   //
   // Generates the complex analytic IF beat signal for one chirp of an FMCW
   // radar observing a single target. The output buffer must be pre-allocated
   // by the caller to exactly p.num_samples elements.
   //
   // Physics:
   //   beat_freq = (B/Tc)*tau + (2*f0*v/c)
   //   tau(t)    = 2*(R + v*t + A*sin(2*pi*fv*t)) / c
   //   s_IF[i]   = exp(j*2*pi*beat_freq*t)   where t = i/fs
   //
   void generate_chirp_if(
       const RadarParams&                p,
       const Target&                     tgt,
       int                               chirp_idx,
       std::vector<std::complex<float>>& out   // PRE-ALLOCATED, size=num_samples
   );

----

``include/csv_export.hpp``
----------------------------

Header-only helper that writes IF signal samples to a three-column CSV file
(``sample,real,imag``). Uses ``std::fixed`` with 8-decimal precision so the
Python FFT receives bit-accurate input. Throws ``std::runtime_error`` with
a descriptive message on file-open failure.

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
       const std::string&                       path,
       const std::vector<std::complex<float>>& sig)
   {
       std::ofstream f(path);
       if (!f) throw std::runtime_error(
                   "write_if_csv: cannot open file: " + path);

       f << "sample,real,imag\n";
       f.precision(8);
       f << std::fixed;

       for (std::size_t i = 0; i < sig.size(); ++i) {
           f << i             << ','
             << sig[i].real() << ','
             << sig[i].imag() << '\n';
       }
   }

----

``src/fmcw_generator.cpp``
----------------------------

.. important::

   The ``out`` buffer is **pre-allocated by the caller**. Never call ``new``,
   ``malloc``, or ``push_back`` inside the per-sample loop. All buffers are
   allocated once at initialisation and reused across frames to avoid heap
   churn on embedded targets.

An ``assert()`` guard on the buffer size catches mismatches between
``p.num_samples`` and the caller's allocation in Debug builds, failing fast
rather than producing silent memory corruption.

.. code-block:: cpp
   :caption: src/fmcw_generator.cpp
   :linenos:

   #include "fmcw_generator.hpp"
   #include <cassert>
   #include <cmath>

   static constexpr double C = 299792458.0; // speed of light (m/s)

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

           // Target range at (slow_t + t) -- includes micro-Doppler
           //   R(t) = R0 + v*(slow_t+t) + A_vib*sin(2*pi*f_vib*(slow_t+t))
           const double total_t = slow_t + t;
           const double R = tgt.range
                          + tgt.velocity * total_t
                          + tgt.vib_amp  * std::sin(
                                2.0 * M_PI * tgt.vib_freq * total_t);

           // Round-trip delay (seconds)
           const double tau = 2.0 * R / C;

           // IF beat frequency (Hz):  f_beat = (B/Tc)*tau + 2*f0*v/c
           const double beat_freq = slope * tau
                                  + 2.0 * p.f0 * tgt.velocity / C;

           // Complex IF sample: exp(j * 2*pi * f_beat * t)
           const double phase = 2.0 * M_PI * beat_freq * t;
           out[i] = std::complex<float>(
               static_cast<float>(std::cos(phase)),
               static_cast<float>(std::sin(phase)));
       }
   }

----

``src/main.cpp``
-----------------

CLI entry point. Configures a 300 GHz THz radar, generates one chirp
for a stationary target at 50 m with 200 Hz engine vibration, writes
``if_signal.csv``, and prints a parameter summary to ``stdout`` so the
output is self-documenting when run in CI logs.

.. code-block:: cpp
   :caption: src/main.cpp
   :linenos:

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

----

Expected ``main()`` Output
---------------------------

.. code-block:: text

   Phase 1: wrote 5000 samples -> if_signal.csv
            f0=300 GHz  B=4 GHz  Tc=100 us  fs=50 MHz
            target: R=50.0 m  v=0.0 m/s  vib=0.200 mm @ 200 Hz
