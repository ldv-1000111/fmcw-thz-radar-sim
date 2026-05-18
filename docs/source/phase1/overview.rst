.. _phase1_overview:

Phase 1 Overview
=================

.. list-table::
   :widths: 30 70
   :stub-columns: 1

   * - **Goal**
     - Build a self-contained C++ library that generates physically accurate
       FMCW IF signals for one or more targets — including THz micro-Doppler
       vibration — validates the output numerically, and exports data for
       independent verification in Python/MATLAB.
   * - **CI gate**
     - All 7 Catch2 tests must pass and the Python range validation must
       print ``PASS`` before tagging ``v0.1.0``.
   * - **Git tag**
     - ``v0.1.0``
   * - **C++ standard**
     - C++17
   * - **Build system**
     - CMake ≥ 3.24
   * - **Test framework**
     - Catch2 v3 (fetched via ``FetchContent``)

Phase 1 Deliverables
---------------------

By the end of Phase 1 the repository must contain:

* ``include/fmcw_generator.hpp`` — ``RadarParams``, ``Target`` structs and
  ``generate_chirp_if()`` declaration
* ``include/csv_export.hpp`` — ``write_if_csv()`` helper
* ``src/fmcw_generator.cpp`` — IF signal implementation
* ``src/main.cpp`` — CLI entry point
* ``tests/test_if_signal.cpp`` — 4 range-accuracy tests (Catch2)
* ``tests/test_micro_doppler.cpp`` — 2 micro-Doppler tests (Catch2)
* ``scripts/plot_if.py`` — Python range-peak validation
* ``CMakeLists.txt`` (root + ``tests/``)
* ``.github/workflows/ci.yml`` — GitHub Actions pipeline

.. important::

   Do **not** advance to Phase 2 until every item in this list is present
   and the CI pipeline reports green on ``main``.
