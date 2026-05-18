# FMCW & TeraHertz Radar Simulation

[![Phase 1 CI](https://github.com/your-org/fmcw-thz-radar-sim/actions/workflows/phase1-ci.yml/badge.svg?branch=main)](https://github.com/your-org/fmcw-thz-radar-sim/actions/workflows/phase1-ci.yml)
[![Phase 2 CI](https://github.com/your-org/fmcw-thz-radar-sim/actions/workflows/phase2-ci.yml/badge.svg?branch=main)](https://github.com/your-org/fmcw-thz-radar-sim/actions/workflows/phase2-ci.yml)
[![Phase 3 CI](https://github.com/your-org/fmcw-thz-radar-sim/actions/workflows/phase3-ci.yml/badge.svg?branch=main)](https://github.com/your-org/fmcw-thz-radar-sim/actions/workflows/phase3-ci.yml)
![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)
![CMake](https://img.shields.io/badge/CMake-3.24%2B-blue)
![License](https://img.shields.io/badge/license-MIT-lightgrey)
![Docs](https://readthedocs.org/projects/fmcw-thz-radar-sim/badge/?version=latest)

**Author:** Luis Viveros · **Date:** May 2026 · **License:** [MIT](LICENSE)

A physics-level FMCW and Terahertz radar simulation built in C++17, with a
2D Range-Doppler processing pipeline (Phase 2) and Yocto cross-compilation
for ADAS embedded targets (Phase 3).

📖 **Full tutorial:** [fmcw-thz-radar-sim.readthedocs.io](https://fmcw-thz-radar-sim.readthedocs.io)

---

## Quick Start

```bash
# 1. Clone
git clone git@github.com:your-org/fmcw-thz-radar-sim.git
cd fmcw-thz-radar-sim

# 2. Build (requires CMake >= 3.24)
cmake -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# 3. Run tests
ctest --test-dir build --output-on-failure -V

# 4. Generate IF signal CSV and validate
./build/radar_sim
python3 scripts/plot_if.py build/if_signal.csv
```

Expected output:

```
Phase 1: wrote 5000 samples -> if_signal.csv
         f0=300 GHz  B=4 GHz  Tc=100 us  fs=50 MHz
         target: R=50.0 m  v=0.0 m/s  vib=0.200 mm @ 200 Hz
PASS  peak @ 49.9904 m  (expected 50.0 m, bin 1334 vs 1334)
Saved  range_spectrum.png
```

---

## Repository Structure

```
fmcw-thz-radar-sim/
├── include/
│   ├── fmcw_generator.hpp   — RadarParams, Target structs + generate_chirp_if()
│   └── csv_export.hpp       — write_if_csv() header-only helper
├── src/
│   ├── fmcw_generator.cpp   — IF signal physics implementation
│   └── main.cpp             — CLI entry point
├── tests/
│   ├── test_if_signal.cpp   — 4 range-accuracy and sanity tests (Catch2)
│   └── test_micro_doppler.cpp — 3 THz micro-Doppler tests (Catch2)
├── scripts/
│   └── plot_if.py           — Python range-peak validation
├── docs/                    — ReadTheDocs Sphinx source
├── .github/workflows/
│   └── phase1-ci.yml        — GitHub Actions: build, test, validate
├── CMakeLists.txt
└── LICENSE
```

---

## Tutorial Progress

This repository follows a three-phase tutorial structure. Each phase has its
own CI workflow file and a live badge at the top of this page. Badges update
automatically when CI runs — no manual docs update required.

### How the badges work

Each badge is a live image served by GitHub. It reflects the CI state the
moment a run finishes — before the docs even rebuild. One workflow file per
phase keeps the badges independent: a failing Phase 2 branch never breaks the
Phase 1 badge.

| Phase | Workflow file | Badge turns green when |
|-------|--------------|------------------------|
| Phase 1 — Physics Engine | `phase1-ci.yml` | All 7 Catch2 tests pass + Python `PASS` |
| Phase 2 — Signal Processing | `phase2-ci.yml` | Range-Doppler + CFAR tests pass |
| Phase 3 — Yocto Deployment | `phase3-ci.yml` | Cross-compile + on-target run passes |

### How the ReadTheDocs pages update

The tutorial docs use two complementary mechanisms to show progress:

**CI badges** on each phase overview page update live from GitHub on every
page load — no docs rebuild needed.

**`.. todo::` directives** mark incomplete sections with a yellow callout
visible to all readers. When a section is finished, the directive is deleted
and the callout disappears on the next RTD build. RTD rebuilds automatically
on every merge to `main`.

A green badge with no yellow callouts means that phase is fully complete.

### Current status

| Phase | Code | Docs |
|-------|------|------|
| **Phase 1 — Physics Engine** | ✅ `v0.1.0` tagged | ✅ Complete |
| **Phase 2 — Signal Processing** | 🔲 Not started | 🔲 Stub pages (todo directives visible) |
| **Phase 3 — Yocto Deployment** | 🔲 Not started | 🔲 Stub pages (todo directives visible) |

### Starting a new phase

```bash
# Create a phase branch
git checkout -b phase-2/signal-processing

# Implement, test locally
cmake --build build --parallel
ctest --test-dir build --output-on-failure -V

# Push — CI triggers automatically
git push -u origin phase-2/signal-processing
# Badge turns yellow while running, green on pass

# After CI is green: merge and tag
git checkout main
git merge --no-ff phase-2/signal-processing
git tag -a v0.2.0 -m "Phase 2 complete: Range-Doppler pipeline, CFAR"
git push origin main --tags
# RTD rebuilds docs, todo directives on phase2/ pages get deleted
```

---

## Phase 1 — Physics Engine (`v0.1.0`)

Generates a complex analytic FMCW IF beat signal for a single target,
including THz micro-Doppler vibration, and exports it to CSV for
independent verification.

### Signal model

```
beat_freq = (B / Tc) * tau  +  2 * f0 * v / c
tau(t)    = 2 * (R  +  v*t  +  A_vib * sin(2*pi*f_vib*t)) / c
s_IF[i]   = exp(j * 2*pi * beat_freq * t)    where  t = i / fs
```

### Default parameters (300 GHz THz system)

| Parameter | Value | Derived |
|-----------|-------|---------|
| `f0` | 300 GHz | lambda = 1 mm |
| `bandwidth` | 4 GHz | delta_r = 3.75 cm |
| `chirp_time` | 100 us | — |
| `fs` | 50 MHz | 5000 samples/chirp |
| Target range | 50 m | Expected bin 1334 |
| Vibration | 0.2 mm @ 200 Hz | Engine idle model |

### Key files

| File | Purpose |
|------|---------|
| `include/fmcw_generator.hpp` | `RadarParams`, `Target` structs; `generate_chirp_if()` declaration |
| `include/csv_export.hpp` | Header-only `write_if_csv()` with 8-decimal fixed precision |
| `src/fmcw_generator.cpp` | IF loop with `assert()` guard, micro-Doppler term, beat-frequency derivation |
| `src/main.cpp` | 300 GHz radar, 50 m target, 200 Hz vibration, structured printf output |
| `tests/test_if_signal.cpp` | Range accuracy (20/50/80 m), buffer size, unit magnitude, slow-time phase |
| `tests/test_micro_doppler.cpp` | Phase variance, zero-amplitude identity, amplitude scaling |
| `scripts/plot_if.py` | FFT peak validation; headless-safe; exits 0/1 for CI |

---

## Dependencies

| Dependency | Version | How it's obtained |
|-----------|---------|-------------------|
| CMake | >= 3.24 | System package / cmake.org |
| Ninja *(optional)* | any | `apt install ninja-build` — faster incremental builds |
| Catch2 | v3.6.0 | `FetchContent` (automatic) |
| numpy + matplotlib | any | `pip install numpy matplotlib` |
| FFTW3 *(Phase 2)* | any | `apt install libfftw3-dev` |

---

## License

MIT — see [LICENSE](LICENSE)

Copyright (c) 2026 Luis Viveros
