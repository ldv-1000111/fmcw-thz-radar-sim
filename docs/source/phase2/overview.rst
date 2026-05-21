.. _phase2_overview:

Phase 2 Overview
=================

.. list-table::
   :widths: 30 70
   :stub-columns: 1

   * - **Prerequisite**
     - Phase 1 tag ``v0.1.0`` on ``main`` and CI green
   * - **Goal**
     - Add a 2D Range-Doppler processing pipeline (FFTW3), a CA-CFAR
       detector, a multi-target CLI mode, a Python Range-Doppler
       visualisation script, and a dedicated Phase 2 CI workflow.
   * - **Git tag**
     - ``v0.2.0``
   * - **New dependency**
     - FFTW3 single-precision (``libfftw3f``) —
       ``sudo apt install libfftw3-dev``
   * - **New source files**
     - ``include/signal_processing.hpp``,
       ``include/cfar.hpp``,
       ``src/signal_processing.cpp``,
       ``src/cfar.cpp``
   * - **New test files**
     - ``tests/test_range_doppler.cpp``,
       ``tests/test_cfar.cpp``
   * - **New CI workflow**
     - ``.github/workflows/phase2-ci.yml``

.. todo::

   Phase 2 implementation is in progress on branch
   ``phase-2/signal-processing``. The pages below will be completed
   once ``v0.2.0`` is tagged on ``main``.

Phase 2 deliverables
----------------------

By the end of Phase 2 the repository must contain:

* ``include/signal_processing.hpp`` — ``compute_range_doppler()``
  declaration
* ``include/cfar.hpp`` — ``cfar_detect()`` declaration
* ``src/signal_processing.cpp`` — FFTW3 2D Range-Doppler implementation
* ``src/cfar.cpp`` — CA-CFAR detector implementation
* Updated ``CMakeLists.txt`` — FFTW3 ``find_package`` and linkage
* ``tests/test_range_doppler.cpp`` — Catch2 tests for the pipeline
* ``tests/test_cfar.cpp`` — Catch2 tests for the CFAR detector
* ``scripts/plot_range_doppler.py`` — Python Range-Doppler visualisation
* ``.github/workflows/phase2-ci.yml`` — CI with FFTW3 install step

.. important::

   Do **not** advance to Phase 3 until all Phase 2 Catch2 tests pass
   and the Python Range-Doppler validation produces a correct map with
   three visible peaks for the three simulated targets.

What Phase 2 adds to Phase 1
------------------------------

Phase 1 generates a single chirp IF signal and validates the range bin
with a 1D FFT. Phase 2 extends this to a full frame:

.. list-table::
   :header-rows: 1
   :widths: 30 35 35

   * - Capability
     - Phase 1
     - Phase 2
   * - Signal generation
     - 1 chirp, 1 target
     - N chirps, multiple targets
   * - Processing
     - 1D Range FFT (Python)
     - 2D Range-Doppler (C++ + FFTW3)
   * - Detection
     - Manual peak inspection
     - CA-CFAR threshold detector
   * - Micro-Doppler
     - Modelled in IF signal
     - Visible as sidebands in R-D map
   * - Validation
     - ``plot_if.py`` range peak
     - ``plot_range_doppler.py`` 2D map
