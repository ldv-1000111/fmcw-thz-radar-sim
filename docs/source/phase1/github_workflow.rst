.. _phase1_github_workflow:

GitHub CI/CD Workflow
======================

Every push to ``main`` or any ``phase-1/**`` branch triggers the pipeline.
The workflow has two jobs: **build-and-test** (configure, build, run Catch2,
generate CSV, Python validation, upload artefacts) and **static-analysis**
(cppcheck).

``.github/workflows/phase1-ci.yml``
-------------------------------------

.. code-block:: yaml
   :caption: .github/workflows/phase1-ci.yml
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
         - name: Checkout repository
           uses: actions/checkout@v4

         - name: Install CMake, Ninja, Python packages  # Ninja is fine on CI runners
           run: |
             sudo apt-get update -qq
             sudo apt-get install -y cmake ninja-build python3-pip
             pip3 install numpy matplotlib

         - name: Configure CMake (Release)
           run: |
             cmake -B build -G Ninja \
               -DCMAKE_BUILD_TYPE=Release \
               -DCMAKE_CXX_STANDARD=17

         - name: Build
           run: cmake --build build --parallel

         - name: Run Catch2 test suite
           working-directory: build
           run: ctest --output-on-failure -V

         - name: Generate IF signal CSV
           # Binary writes if_signal.csv next to itself (build/if_signal.csv)
           # because main.cpp uses filesystem::path(argv[0]).parent_path()
           working-directory: build
           run: ./radar_sim

         - name: Python range-peak validation
           run: python3 scripts/plot_if.py build/if_signal.csv

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
             cppcheck --enable=all \
                      --error-exitcode=1 \
                      --suppress=missingIncludeSystem \
                      --suppress=unmatchedSuppression \
                      --std=c++17 \
                      -I include \
                      src/

Phase 1 Push Checklist
-----------------------

.. code-block:: bash

   # Create branch
   git checkout -b phase-1/physics-engine

   # Stage all Phase 1 files
   git add CMakeLists.txt CMakePresets.json README.md \
           .readthedocs.yaml .gitignore \
           include/ src/ tests/ scripts/ docs/ \
           .github/workflows/phase1-ci.yml

   # Commit
   git commit -m "feat(phase1): FMCW IF physics engine, Catch2 tests, RTD docs, CI"

   # Verify locally before pushing
   cmake -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
   cmake --build build --parallel
   ctest --test-dir build --output-on-failure -V
   ./build/radar_sim
   python3 scripts/plot_if.py build/if_signal.csv

   # Push and open Pull Request
   git push -u origin phase-1/physics-engine
   # → CI triggers automatically; both jobs must be green before merging

   # After green CI: merge and tag
   git checkout main
   git pull origin main
   git merge --no-ff phase-1/physics-engine
   git tag -a v0.1.0 -m "Phase 1 complete: IF physics engine, tests, RTD docs, CI"
   git push origin main --tags

Branch Protection — Practical Notes
--------------------------------------

GitHub branch protection rulesets require the status check names to match
**exactly** as GitHub registers them, including the event context suffix
(e.g. ``(pull_request)``). The registered name depends on the workflow
``name:`` field and the job ``name:`` field combined.

For this repository the correct required check names are:

* ``Phase 1 CI / Build · Test · Validate (Ubuntu latest)``
* ``Phase 1 CI / cppcheck static analysis``

.. note::

   For a solo tutorial project, branch protection adds workflow overhead
   without meaningful safety benefit — you are the only contributor.
   The CI pipeline still runs on every push and surfaces failures; it
   simply does not block a merge if checks fail.

   If you want to enforce required checks, use **Settings → Rulesets**
   (not the legacy *Branch protection rules* interface), set the target
   branch to ``main``, and add the two check names above. Do **not** use
   the bare job ids ``build-and-test`` and ``static-analysis`` — GitHub
   will not match them to the running workflow jobs.
