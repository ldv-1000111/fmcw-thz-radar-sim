#!/usr/bin/env python3
"""
Phase 2 validation — Range-Doppler map visualisation and peak verification.

Usage:
    python3 scripts/plot_range_doppler.py [path/to/range_doppler.csv]

Default CSV path: build/range_doppler.csv
Saves range_doppler_map.png in the current directory.
Prints PASS/FAIL and exits with code 0/1 for CI.

CSV format (written by radar_sim Phase 2):
    doppler_bin,range_bin,magnitude
"""

import csv
import sys
import os
import math

# ── Parameters — must match Phase 2 main.cpp ─────────────────────────────
C          = 299_792_458.0   # speed of light (m/s)
F0         = 300e9           # start frequency (Hz)
BW         = 4e9             # bandwidth (Hz)
TC         = 100e-6          # chirp time (s)
FS         = 50e6            # IF sample rate (Hz)
NS         = 256             # num_samples
NC         = 256             # num_chirps

# Expected targets (range m, velocity m/s) — must match main.cpp exactly
# bin_res = c / (2 * f0 * NC * TC) = 0.01953 m/s/bin  (NC=256)
TARGETS = [
    (20.0,  0.000),   # bin   0
    (50.0,  0.625),   # bin  32
    (80.0,  4.375),   # bin 224
]

RANGE_TOL_BINS   = 2   # allowed range bin error
DOPPLER_TOL_BINS = 2   # allowed Doppler bin error

# ── Helper functions ──────────────────────────────────────────────────────

def range_bin(R):
    tau    = 2.0 * R / C
    f_beat = (BW / TC) * tau
    return int(round(f_beat * NS / FS))

def doppler_bin(v):
    f_d = 2.0 * F0 * v / C
    bin_idx = round(f_d * NC * TC)
    return int(bin_idx % NC)  # wrap negative bins to upper half of FFT

# ── Load CSV ──────────────────────────────────────────────────────────────
csv_path = sys.argv[1] if len(sys.argv) > 1 else "build/range_doppler.csv"

if not os.path.exists(csv_path):
    print(f"FAIL  file not found: {csv_path}", file=sys.stderr)
    sys.exit(1)

try:
    import numpy as np
except ImportError:
    print("FAIL  numpy not installed — run: sudo apt install python3-numpy",
          file=sys.stderr)
    sys.exit(1)

rd_map = {}
with open(csv_path, newline="") as fh:
    reader = csv.DictReader(fh)
    for row in reader:
        d = int(row["doppler_bin"])
        r = int(row["range_bin"])
        m = float(row["magnitude"])
        rd_map[(d, r)] = m

if not rd_map:
    print("FAIL  CSV is empty", file=sys.stderr)
    sys.exit(1)

# Reconstruct 2D array
d_max = max(k[0] for k in rd_map) + 1
r_max = max(k[1] for k in rd_map) + 1
grid  = np.zeros((d_max, r_max), dtype=np.float32)
for (d, r), m in rd_map.items():
    grid[d, r] = m

# ── Validate peaks ────────────────────────────────────────────────────────
all_pass = True
for R, v in TARGETS:
    exp_r = range_bin(R)
    exp_d = doppler_bin(v)

    # Search neighbourhood around expected bin
    r_lo = max(0, exp_r - RANGE_TOL_BINS)
    r_hi = min(r_max - 1, exp_r + RANGE_TOL_BINS)
    d_lo = max(0, exp_d - DOPPLER_TOL_BINS)
    d_hi = min(d_max - 1, exp_d + DOPPLER_TOL_BINS)

    region   = grid[d_lo:d_hi+1, r_lo:r_hi+1]
    peak_val = float(np.max(region))

    # Local noise: median of non-DC Doppler bins at this range bin.
    # Bin 0 (DC) is always large (= mean of slow-time sequence * NC)
    # and must be excluded to get a meaningful noise floor estimate
    # for moving targets at non-zero Doppler bins.
    col = grid[:, exp_r]
    noise_bins = col[1:d_max // 2]  # positive Doppler frequencies, no DC
    local_noise = float(np.median(noise_bins))
    if local_noise < 1e-6:
        local_noise = float(np.median(col))
    snr = peak_val / local_noise if local_noise > 0 else 0.0

    if snr > 3.0:
        print(f"PASS  target R={R:.0f}m v={v:+.3f}m/s  "
              f"exp bin=({exp_d},{exp_r})  SNR={snr:.1f}")
    else:
        print(f"FAIL  target R={R:.0f}m v={v:+.3f}m/s  "
              f"exp bin=({exp_d},{exp_r})  SNR={snr:.1f} (need >3)",
              file=sys.stderr)
        all_pass = False

# ── Plot ──────────────────────────────────────────────────────────────────
try:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    # Range and velocity axes
    range_axis  = np.arange(r_max) * (C * TC / (2.0 * BW))
    vel_res     = (3e8 / F0) / (2.0 * NC * TC)
    doppler_axis = np.arange(d_max) * vel_res

    fig, ax = plt.subplots(figsize=(10, 5))
    im = ax.imshow(
        grid,
        aspect="auto",
        origin="lower",
        extent=[range_axis[0], range_axis[-1],
                doppler_axis[0], doppler_axis[-1]],
        cmap="viridis"
    )
    plt.colorbar(im, ax=ax, label="Magnitude")

    for R, v in TARGETS:
        ax.axvline(R, color="red",    ls="--", lw=0.8, alpha=0.7,
                   label=f"R={R:.0f}m")
        ax.axhline(v, color="orange", ls=":",  lw=0.8, alpha=0.7)

    ax.set_xlabel("Range (m)")
    ax.set_ylabel("Velocity (m/s)")
    ax.set_title(
        f"Phase 2 — Range-Doppler Map  "
        f"(f₀={F0/1e9:.0f} GHz, B={BW/1e9:.0f} GHz, "
        f"N_c={NC}, N_s={NS})"
    )
    handles = [plt.Line2D([0],[0], color="red",    ls="--", label="Expected range"),
               plt.Line2D([0],[0], color="orange", ls=":",  label="Expected velocity")]
    ax.legend(handles=handles, fontsize=9)
    fig.tight_layout()

    out_png = "range_doppler_map.png"
    fig.savefig(out_png, dpi=150)
    print(f"Saved  {out_png}")

except ImportError:
    print("Note:  matplotlib not installed — skipping plot")

sys.exit(0 if all_pass else 1)
