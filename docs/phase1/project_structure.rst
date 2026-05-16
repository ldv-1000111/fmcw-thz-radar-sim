.. _phase1_project_structure:

Project Structure
==================

The repository is laid out so that each phase adds files in a predictable
location without touching files from earlier phases.

.. code-block:: text

   fmcw-thz-radar-sim/
   ├── .github/
   │   └── workflows/
   │       └── ci.yml                  ← GitHub Actions (Phase 1)
   ├── CMakeLists.txt                  ← root build file
   ├── README.md
   ├── LICENSE
   │
   ├── include/                        ← public headers
   │   ├── fmcw_generator.hpp          ← RadarParams, Target, generate_chirp_if()
   │   └── csv_export.hpp              ← write_if_csv() helper
   │
   ├── src/
   │   ├── fmcw_generator.cpp          ← IF signal implementation
   │   └── main.cpp                    ← CLI entry point
   │
   ├── tests/
   │   ├── CMakeLists.txt              ← Catch2 test target
   │   ├── test_if_signal.cpp          ← range accuracy tests  (Phase 1)
   │   ├── test_micro_doppler.cpp      ← THz vibration tests   (Phase 1)
   │   ├── test_range_doppler.cpp      ← 2D FFT tests          (Phase 2, TODO)
   │   └── test_cfar.cpp               ← CFAR detector tests   (Phase 2, TODO)
   │
   └── scripts/
       ├── plot_if.py                  ← Phase 1 Python validation
       └── plot_range_doppler.py       ← Phase 2 Python validation  (TODO)

.. note::

   Files marked ``(TODO)`` are placeholders for Phase 2. Create them as empty
   files now so the directory structure is committed from day one:

   .. code-block:: bash

      touch tests/test_range_doppler.cpp
      touch tests/test_cfar.cpp
      touch scripts/plot_range_doppler.py

   They will be excluded from the build until Phase 2's ``CMakeLists.txt``
   adds them.

Phase Tags and File Ownership
-------------------------------

.. list-table::
   :header-rows: 1
   :widths: 15 25 60

   * - Git Tag
     - Files Added
     - Purpose
   * - ``v0.1.0``
     - ``include/``, ``src/``, ``tests/test_if_*.cpp``, ``tests/test_micro_*.cpp``,
       ``scripts/plot_if.py``, ``CMakeLists.txt``, ``.github/workflows/ci.yml``
     - Phase 1 complete — physics engine, tests, CI
   * - ``v0.2.0``
     - ``src/signal_processing.cpp``, ``include/signal_processing.hpp``,
       ``tests/test_range_doppler.cpp``, ``tests/test_cfar.cpp``,
       ``scripts/plot_range_doppler.py``
     - Phase 2 complete — Range-Doppler pipeline, CFAR
   * - ``v0.3.0``
     - ``poky-layer/meta-adas-radar/``, updated ``CMakeLists.txt``
     - Phase 3 complete — Yocto layer, cross-compilation, on-target profiling
