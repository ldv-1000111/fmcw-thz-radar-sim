.. _phase1_github_workflow:

GitHub CI/CD Workflow
======================

Every push to ``main`` or any ``phase-1/**`` branch triggers the pipeline.
A pull request to ``main`` **cannot be merged** until all jobs pass.

``.github/workflows/ci.yml``
------------------------------

.. code-block:: yaml
   :caption: .github/workflows/ci.yml
   :linenos:

   name: Phase 1 CI

   on:
     push:
       branches: [ "main", "phase-1/**" ]
     pull_request:
       branches: [ "main" ]

   jobs:
     build-and-test:
       name: Build · Test · Validate (Ubuntu latest)
       runs-on: ubuntu-latest

       steps:
         # ── 1. Checkout ──────────────────────────────────────
         - name: Checkout repository
           uses: actions/checkout@v4

         # ── 2. Dependencies ──────────────────────────────────
         - name: Install CMake, Ninja, Python packages
           run: |
             sudo apt-get update -qq
             sudo apt-get install -y cmake ninja-build python3-pip
             pip3 install numpy matplotlib

         # ── 3. Configure ────────────────────────────────────
         - name: Configure CMake (Release)
           run: |
             cmake -B build -G Ninja \
               -DCMAKE_BUILD_TYPE=Release \
               -DCMAKE_CXX_STANDARD=17

         # ── 4. Build ─────────────────────────────────────────
         - name: Build
           run: cmake --build build --parallel

         # ── 5. Unit tests ────────────────────────────────────
         - name: Run Catch2 test suite
           working-directory: build
           run: ctest --output-on-failure -V

         # ── 6. Generate IF signal CSV ────────────────────────
         - name: Run radar_sim (generate if_signal.csv)
           working-directory: build
           run: ./radar_sim

         # ── 7. Python range validation ───────────────────────
         - name: Python range-peak validation
           run: |
             cp build/if_signal.csv scripts/
             python3 scripts/plot_if.py scripts/if_signal.csv

         # ── 8. Upload artefacts ──────────────────────────────
         - name: Upload validation artefacts
           uses: actions/upload-artifact@v4
           with:
             name: phase1-validation-${{ github.sha }}
             path: |
               build/if_signal.csv
               range_spectrum.png
             retention-days: 14

     static-analysis:
       name: cppcheck static analysis
       runs-on: ubuntu-latest
       steps:
         - uses: actions/checkout@v4
         - run: sudo apt-get install -y cppcheck
         - name: Run cppcheck
           run: |
             cppcheck --enable=all --error-exitcode=1 \
               --suppress=missingIncludeSystem \
               --std=c++17 \
               -I include src/

Phase 1 Push Checklist
-----------------------

Follow this sequence for the first clean push:

.. code-block:: bash

   # ── Create branch ────────────────────────────────────────────
   git checkout -b phase-1/physics-engine

   # ── Stage all Phase 1 files ──────────────────────────────────
   git add CMakeLists.txt include/ src/ tests/ scripts/ .github/

   # ── Commit ───────────────────────────────────────────────────
   git commit -m "feat(phase1): FMCW IF physics engine, Catch2 tests, CI"

   # ── Local verification before pushing ────────────────────────
   cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
   cmake --build build --parallel
   ctest --test-dir build --output-on-failure -V

   # ── Push and open Pull Request ───────────────────────────────
   git push -u origin phase-1/physics-engine
   # → Check Actions tab — CI must be green before merging

   # ── After green CI: merge and tag ────────────────────────────
   git checkout main
   git merge --no-ff phase-1/physics-engine
   git tag -a v0.1.0 -m "Phase 1 complete: IF physics engine, tests, CI"
   git push origin main --tags

.. important::

   Enable **branch protection** on ``main`` in *Settings → Branches →
   Branch protection rules*:

   * ✅ Require status checks to pass before merging
   * ✅ Select the ``build-and-test`` and ``static-analysis`` jobs
   * ✅ Require branches to be up to date before merging

   This prevents any broken physics code from ever landing on ``main``.
