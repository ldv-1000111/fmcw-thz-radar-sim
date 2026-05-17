.. _phase1_repo_setup:

Repository & Project Setup
===========================

The repository already exists on GitHub. Start by cloning it locally and
wiring in the CMake build **before writing a single line of radar code**.
This order matters: CI should be green on an empty project so that any
future failure is always attributable to new code.

Step 1 — Clone and Scaffold
-----------------------------

.. code-block:: bash

   # SSH (recommended)
   git clone git@github.com:your-org/fmcw-thz-radar-sim.git
   cd fmcw-thz-radar-sim

   # Create the full directory skeleton in one shot
   mkdir -p include src tests scripts .github/workflows

   # Create placeholder files so git tracks the directories
   touch include/fmcw_generator.hpp include/csv_export.hpp
   touch src/fmcw_generator.cpp src/main.cpp
   touch tests/CMakeLists.txt
   touch tests/test_if_signal.cpp tests/test_micro_doppler.cpp
   touch tests/test_range_doppler.cpp tests/test_cfar.cpp
   touch scripts/plot_if.py scripts/plot_range_doppler.py

.. important::

   Before running any ``cmake`` command, you must first populate the source
   files. The ``touch`` commands above created empty placeholders — the linker
   will fail with ``undefined reference to 'main'`` if you try to build before
   the content is in place.

   **Complete** :ref:`phase1_cpp_implementation` now, which provides the full
   source for every file listed above:

   * ``include/fmcw_generator.hpp`` — ``RadarParams``, ``Target``, declaration
   * ``include/csv_export.hpp`` — ``write_if_csv()``
   * ``src/fmcw_generator.cpp`` — IF signal implementation
   * ``src/main.cpp`` — CLI entry point

   Then return here and proceed with Step 2.

Step 2 — Add CMakeLists and Confirm a Clean Build
--------------------------------------------------

With all source files populated, create the files described in
:ref:`phase1_cmake_build`, then verify:

.. code-block:: bash

   cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
   cmake --build build --parallel
   # Expected: radar_sim binary produced, 0 errors

Step 3 — Add the GitHub Actions Workflow
-----------------------------------------

Create ``.github/workflows/ci.yml`` as described in
:ref:`phase1_github_workflow`, then commit and push to confirm the pipeline
is green before any physics code is added:

.. code-block:: bash

   git add .
   git commit -m "chore: scaffold project, stub CMake, empty CI"
   git push origin main
   # → Visit the Actions tab — both jobs must be green before proceeding

Step 4 — Implement, Test, Commit
----------------------------------

Follow the Phase 1 pages in this order, keeping the test suite green
at every push:

#. :ref:`phase1_fmcw_theory` — understand the signal model
#. :ref:`phase1_cpp_implementation` — **write all source files first** (required before Step 2 above)
#. :ref:`phase1_cmake_build` — finalise ``CMakeLists.txt``
#. :ref:`phase1_testing` — write and run Catch2 tests
#. :ref:`phase1_python_validation` — verify the range peak in Python
#. :ref:`phase1_github_workflow` — push, CI green, tag ``v0.1.0``

.. tip::

   Use short, descriptive commit messages following the
   `Conventional Commits <https://www.conventionalcommits.org>`_ format:
   ``feat(phase1): add generate_chirp_if() implementation``.
