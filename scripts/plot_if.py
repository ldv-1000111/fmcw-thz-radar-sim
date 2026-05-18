#!/usr/bin/env python3
"""
Phase 1 validation — verify the range-bin peak of if_signal.csv.

Usage:
    python3 scripts/plot_if.py [path/to/if_signal.csv]

Default CSV path: if_signal.csv (run from the build directory or pass the path).
Saves range_spectrum.png in the current directory.
Prints PASS/FAIL to stdout and exits with code 0/1 so CI can check it.
"""

import csv
import sys
import os

# ── Parameters — must match main.cpp exactly ──────────────────────────────
C          = 299_792_458.0   # speed of light (m/s)
F0         = 300e9           # start frequency (Hz)
BANDWIDTH  = 4e9             # sweep bandwidth (Hz)
CHIRP_TIME = 100e-6          # chirp duration (s)
FS         = 50e6            # IF sampling rate (Hz)
TARGET_R   = 50.0            # expected target range (m)

# ── Load CSV ──────────────────────────────────────────────────────────────
csv_path = sys.argv[1] if len(sys.argv) > 1 else "if_signal.csv"

if not os.path.exists(csv_path):
    print(f"FAIL  file not found: {csv_path}", file=sys.stderr)
    sys.exit(1)

with open(csv_path, newline="") as fh:
    reader = csv.DictReader(fh)
    rows   = list(reader)

if not rows:
    print("FAIL  CSV is empty", file=sys.stderr)
    sys.exit(1)

try:
    import numpy as np
except ImportError:
    print("FAIL  numpy not installed — run: pip install numpy matplotlib",
          file=sys.stderr)
    sys.exit(1)

signal = np.array([
    complex(float(r["real"]), float(r["imag"])) for r in rows
])
N = len(signal)

# ── FFT & range axis ──────────────────────────────────────────────────────
spectrum  = np.abs(np.fft.fft(signal))[:N // 2]
freqs     = np.fft.fftfreq(N, 1.0 / FS)[:N // 2]
ranges_m  = freqs * C * CHIRP_TIME / (2.0 * BANDWIDTH)

peak_idx  = int(np.argmax(spectrum))
peak_r    = ranges_m[peak_idx]

# Expected bin for a stationary target at TARGET_R
expected_bin = int(round(
    (BANDWIDTH / CHIRP_TIME) * (2.0 * TARGET_R / C) * N / FS
))

# ── Validation ────────────────────────────────────────────────────────────
ok = abs(peak_idx - expected_bin) <= 1

if ok:
    print(f"PASS  peak @ {peak_r:.4f} m  "
          f"(expected {TARGET_R} m, "
          f"bin {peak_idx} vs {expected_bin})")
else:
    print(f"FAIL  peak @ {peak_r:.4f} m  "
          f"(expected {TARGET_R} m, "
          f"bin {peak_idx} vs {expected_bin}  delta={abs(peak_idx-expected_bin)})",
          file=sys.stderr)

# ── Plot (save only — headless-safe for CI) ───────────────────────────────
try:
    import matplotlib
    matplotlib.use("Agg")   # no display needed
    import matplotlib.pyplot as plt

    fig, ax = plt.subplots(figsize=(10, 4))
    ax.plot(ranges_m, spectrum, lw=0.8, color="#2980b9",
            label="IF magnitude spectrum")
    ax.axvline(TARGET_R, color="#e74c3c", ls="--", lw=1.2,
               label=f"Expected target {TARGET_R} m")
    ax.axvline(peak_r, color="#27ae60", ls=":", lw=1.0,
               label=f"Detected peak {peak_r:.2f} m")
    ax.set_xlabel("Range (m)")
    ax.set_ylabel("Magnitude")
    ax.set_title(
        f"Phase 1 — Range FFT of IF Signal  "
        f"(f₀={F0/1e9:.0f} GHz, B={BANDWIDTH/1e9:.0f} GHz)"
    )
    ax.legend(fontsize=10)
    fig.tight_layout()

    out_png = "range_spectrum.png"
    fig.savefig(out_png, dpi=150)
    print(f"Saved  {out_png}")

except ImportError:
    print("Note:  matplotlib not installed — skipping plot (install: pip install matplotlib)")

sys.exit(0 if ok else 1)
