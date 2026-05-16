.. _phase1_repo_setup:

Repository & Project Setup
===========================

Start by initialising the GitHub repository and wiring in the CMake build
**before writing a single line of radar code**. This order matters: CI should
be green on an empty project so that any future failure is always attributable
to new code.

Step 1 — Create the GitHub Repository
---------------------------------------

1. Go to https://github.com/new
2. Repository name: ``fmcw-thz-radar-sim``
3. Description: ``FMCW & TeraHertz Radar Simulation — C++17 + Yocto``
4. Visibility: Public (required for free GitHub Actions minutes)
5. Initialise with: ``README.md``, ``.gitignore`` (C++ template), MIT licence
6. Click **Create repository**

Step 2 — Clone and Scaffold
-----------------------------

.. code-block:: bash

   git clone https://github.com/your-org/fmcw-thz-radar-sim.git
   cd fmcw-thz-radar-sim

   # Create directory skeleton
   mkdir -p include src tests scripts .github/workflows

Step 3 — Add a Stub CMakeLists and Confirm a Clean Build
----------------------------------------------------------

Create the files described in :ref:`phase1_cmake_build`, then verify:

.. code-block:: bash

   cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
   cmake --build build --parallel
   # Expected: radar_sim binary produced, 0 errors

Step 4 — Add the GitHub Actions Workflow
-----------------------------------------

Create ``.github/workflows/ci.yml`` as described in
:ref:`phase1_github_workflow` and push:

.. code-block:: bash

   git add .
   git commit -m "chore: scaffold project, stub CMake, empty CI"
   git push origin main
   # → Visit Actions tab — pipeline should be green before physics code is added

Step 5 — Implement, Test, Commit
----------------------------------

Follow the remaining Phase 1 pages in order, keeping the test suite green
at every push:

#. :ref:`phase1_fmcw_theory` — understand the signal model
#. :ref:`phase1_cpp_implementation` — write ``fmcw_generator.*`` and ``main.cpp``
#. :ref:`phase1_cmake_build` — finalise ``CMakeLists.txt``
#. :ref:`phase1_testing` — write and run Catch2 tests
#. :ref:`phase1_python_validation` — verify the range peak in Python
#. :ref:`phase1_github_workflow` — push, CI green, tag ``v0.1.0``

.. tip::

   Use short, descriptive commit messages following the
   `Conventional Commits <https://www.conventionalcommits.org>`_ format:
   ``feat(phase1): add generate_chirp_if() implementation``.
