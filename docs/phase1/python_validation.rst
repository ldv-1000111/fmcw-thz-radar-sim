.. _phase1_python_validation:

Python Range Validation
========================

After running ``./build/radar_sim`` to generate ``if_signal.csv``, run
``scripts/plot_if.py`` to verify the FFT range peak numerically and
produce a matplotlib figure.

.. code-block:: python
   :caption: scripts/plot_if.py
   :linenos:

   #!/usr/bin/env python3
   """Phase 1 validation: verify range-bin peak from if_signal.csv."""
   import numpy as np
   import matplotlib
   matplotlib.use("Agg")   # headless-safe for CI
   import matplotlib.pyplot as plt
   import csv, sys

   C          = 299_792_458.0
   F0         = 300e9        # Hz  — must match main.cpp
   BANDWIDTH  = 4e9          # Hz
   CHIRP_TIME = 100e-6       # s
   FS         = 50e6         # Hz
   TARGET_R   = 50.0         # m   — must match main.cpp

   # ── Load CSV ──────────────────────────────────────────────────
   csv_path = sys.argv[1] if len(sys.argv) > 1 else "if_signal.csv"
   with open(csv_path) as fh:
       rows = list(csv.DictReader(fh))

   signal = np.array([complex(float(r["real"]), float(r["imag"])) for r in rows])
   N      = len(signal)

   # ── FFT & range axis ─────────────────────────────────────────
   spectrum = np.abs(np.fft.fft(signal))[:N // 2]
   freqs    = np.fft.fftfreq(N, 1.0 / FS)[:N // 2]
   ranges   = freqs * C * CHIRP_TIME / (2.0 * BANDWIDTH)

   peak_idx  = int(np.argmax(spectrum))
   peak_r    = ranges[peak_idx]
   expected  = int(round((BANDWIDTH / CHIRP_TIME) * (2 * TARGET_R / C) * N / FS))

   # ── Assertion ────────────────────────────────────────────────
   assert abs(peak_idx - expected) <= 1, (
       f"FAIL: peak bin {peak_idx}, expected {expected} "
       f"(target {TARGET_R} m, got {peak_r:.3f} m)"
   )
   print(f"PASS  peak @ {peak_r:.3f} m  "
         f"(expected {TARGET_R} m, bin {peak_idx} vs {expected})")

   # ── Plot ─────────────────────────────────────────────────────
   fig, ax = plt.subplots(figsize=(10, 4))
   ax.plot(ranges, spectrum, lw=0.8, color="#2980b9", label="IF magnitude spectrum")
   ax.axvline(TARGET_R, color="#e74c3c", ls="--", lw=1.2,
              label=f"Target {TARGET_R} m")
   ax.set_xlabel("Range (m)")
   ax.set_ylabel("Magnitude")
   ax.set_title("Phase 1 — Range FFT of IF Signal (300 GHz THz Radar)")
   ax.legend()
   fig.tight_layout()
   fig.savefig("range_spectrum.png", dpi=150)
   print("Saved  range_spectrum.png")

Running the Script
-------------------

.. code-block:: bash

   # 1. Generate the CSV
   ./build/radar_sim

   # 2. Install Python dependencies (first time only)
   pip3 install numpy matplotlib

   # 3. Run validation
   python3 scripts/plot_if.py
   # Expected output:
   # PASS  peak @ 50.000 m  (expected 50.0 m, bin 133 vs 133)
   # Saved  range_spectrum.png

Expected Output Figure
-----------------------

The saved ``range_spectrum.png`` should show:

* A single sharp peak near 50 m (bin 133 for the parameters in ``main.cpp``)
* No significant secondary peaks
* A red dashed vertical line at exactly 50 m

.. note::

   The ``matplotlib.use("Agg")`` call at the top of the script makes it
   safe to run in headless CI environments (no display required).
   The GitHub Actions workflow uploads ``range_spectrum.png`` as an artefact
   so you can inspect it without downloading.
