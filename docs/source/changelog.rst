.. _changelog:

Changelog
==========

All notable changes are documented here following
`Keep a Changelog <https://keepachangelog.com>`_ conventions.

v0.1.0 — May 2026
-------------------

**Added**

* ``include/fmcw_generator.hpp`` — ``RadarParams``, ``Target`` structs and
  ``generate_chirp_if()`` declaration
* ``include/csv_export.hpp`` — ``write_if_csv()`` helper
* ``src/fmcw_generator.cpp`` — full IF signal implementation with THz
  micro-Doppler support
* ``src/main.cpp`` — CLI entry point (300 GHz THz, 50 m target, 200 Hz vib)
* ``tests/test_if_signal.cpp`` — 5 Catch2 range-accuracy and sanity tests
* ``tests/test_micro_doppler.cpp`` — 2 Catch2 micro-Doppler tests
* ``scripts/plot_if.py`` — Python range-peak validation with matplotlib
* ``CMakeLists.txt`` (root + ``tests/``) — Catch2 via ``FetchContent``
* ``.github/workflows/ci.yml`` — GitHub Actions: build, test, validate,
  cppcheck static analysis, artefact upload

v0.2.0 — *Planned*
--------------------

* Range-Doppler pipeline with FFTW3
* CA-CFAR detector
* Python Range-Doppler visualisation script

v0.3.0 — *Planned*
--------------------

* Yocto ``meta-adas-radar`` layer
* BitBake recipe (``radar-sim_1.0.bb``)
* Cross-compilation for NXP S32G, Renesas R-Car H3, Raspberry Pi 5
