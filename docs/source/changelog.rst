.. _changelog:

Changelog
==========

All notable changes are documented here following
`Keep a Changelog <https://keepachangelog.com>`_ conventions.

----

v0.1.0 — May 2026
-------------------

Phase 1 complete. All 7 Catch2 tests passing. Python range validation
prints ``PASS``. CI pipeline green on ``main``.

**Added — Source Files**

* ``include/fmcw_generator.hpp`` — ``RadarParams`` and ``Target`` POD
  structs; ``generate_chirp_if()`` declaration with full physics comments
  describing the beat-frequency and micro-Doppler equations
* ``include/csv_export.hpp`` — ``write_if_csv()`` helper with ``std::fixed``
  precision and ``std::runtime_error`` on file-open failure
* ``src/fmcw_generator.cpp`` — IF signal implementation with ``assert()``
  buffer-size guard, per-line physics comments, THz micro-Doppler vibration
  term, and explicit ``total_t`` variable for clarity
* ``src/main.cpp`` — CLI entry point with structured parameter printout
  (frequency, bandwidth, chirp time, sample rate, target parameters);
  expected range bin noted in comments (bin 1334 for the default 50 m target)

**Added — Tests**

* ``tests/test_if_signal.cpp`` — 4 Catch2 tests:

  - Range bin accuracy within ±1 bin at 20 m, 50 m, 80 m (brute-force DFT)
  - Output buffer size equals ``p.num_samples``
  - All samples have unit magnitude (``|exp(j*phase)| = 1``)
  - Chirp index shifts slow-time phase at sample ``N/2`` (see bug fix below)

* ``tests/test_micro_doppler.cpp`` — 3 Catch2 tests:

  - Vibration increases inter-chirp phase difference variance (16 chirps)
  - Zero ``vib_amp`` produces output identical to no-vibration target
  - Larger ``vib_amp`` (0.2 mm) produces greater phase spread than
    smaller (0.1 mm)

* ``tests/CMakeLists.txt`` — Catch2 target with ``catch_discover_tests``
  and per-compiler warning flags

**Added — Tooling**

* ``CMakeLists.txt`` — root build: ``fmcw_core`` static library,
  ``radar_sim`` executable, Catch2 v3.6.0 via ``FetchContent``
* ``scripts/plot_if.py`` — headless-safe Python validation; exits with
  code 0/1 for CI; saves ``range_spectrum.png``; accepts CSV path as
  optional ``argv[1]``
* ``.github/workflows/phase1-ci.yml`` — two jobs: ``build-and-test``
  (configure, build, ctest, run binary, Python validation, upload
  artefacts) and ``static-analysis`` (cppcheck)
* ``README.md`` — live Phase 1 CI badge, quick-start commands,
  project structure table

**Bug Fixes (caught during initial build verification)**

.. warning::

   These bugs were in the **test code**, not the physics implementation.
   ``generate_chirp_if()`` produced correct output from the first commit.

* **Test 4 — slow-time phase comparison at wrong sample index**

  The original test compared ``std::arg(chirp0[0])`` vs ``std::arg(chirp1[0])``.
  At fast-time index ``i = 0``, the time ``t = i / fs = 0``, which makes
  ``phase = 2 * pi * f_beat * 0 = 0`` for **any** chirp regardless of
  slow-time offset. The assertion ``|phase0 - phase1| > 1e-3`` always
  failed because both were exactly 0.

  *Fix:* compare at index ``mid = num_samples / 2`` where ``t > 0`` and
  the phase is a meaningful function of the chirp's slow-time offset.

* **Tests 5 & 7 — phase variance collapsed to zero due to t=0 and wrap**

  The original micro-Doppler tests collected ``std::arg(buf[0])`` across
  8 chirps and computed variance. Because sample 0 gives ``phase = 0``
  always, both the vibrating and non-vibrating targets produced a
  sequence of all-zeros, giving ``variance = 0`` for both — making the
  ``var_vib > var_flat`` assertion trivially false.

  *Fix:* replaced raw phase collection with **inter-chirp phase
  differences** sampled at ``buf[N/2]``. Differences remove the constant
  Doppler slope and expose the vibration modulation without the ``t = 0``
  problem or ``[-pi, pi]`` wrapping artifacts. Chirp count increased from
  8 to 16 to give the variance estimator more samples.

----

v0.2.0 — *Planned*
--------------------

* ``include/signal_processing.hpp`` / ``src/signal_processing.cpp`` —
  FFTW3 2D Range-Doppler pipeline
* ``include/cfar.hpp`` / ``src/cfar.cpp`` — CA-CFAR detector
* ``tests/test_range_doppler.cpp`` / ``tests/test_cfar.cpp``
* ``scripts/plot_range_doppler.py``
* ``.github/workflows/phase2-ci.yml`` with FFTW3 installation step

----

v0.3.0 — *Planned*
--------------------

* ``poky-layer/meta-adas-radar/`` — Yocto custom meta-layer
* ``poky-layer/recipes-apps/radar-sim/radar-sim_1.0.bb`` — BitBake recipe
* Cross-compilation targets: NXP S32G, Renesas R-Car H3, Raspberry Pi 5
* ``.github/workflows/phase3-ci.yml``
