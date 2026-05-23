.. _phase2_github_workflow:

Phase 2 GitHub Workflow
========================

Phase 2 adds a dedicated CI workflow that installs FFTW3 and runs the
Range-Doppler and CFAR test suites. The Phase 1 badge is unaffected —
each phase has its own workflow file and badge.

``.github/workflows/phase2-ci.yml``
-------------------------------------

.. todo::

   ``phase2-ci.yml`` will be committed as part of the
   ``phase-2/signal-processing`` branch. The content is shown below
   for reference.

.. code-block:: yaml
   :caption: .github/workflows/phase2-ci.yml
   :linenos:

   name: Phase 2 CI

   on:
     push:
       branches: [ "main", "phase-2/**" ]
     pull_request:
       branches: [ "main" ]

   jobs:
     build-and-test:
       name: Build · Test · Validate (Ubuntu latest)
       runs-on: ubuntu-latest

       steps:
         - name: Checkout repository
           uses: actions/checkout@v4

         - name: Install CMake, Ninja, FFTW3, Python packages
           run: |
             sudo apt-get update -qq
             sudo apt-get install -y cmake ninja-build libfftw3-dev \
                                     python3-pip
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

         - name: Generate Range-Doppler map
           working-directory: build
           run: ./radar_sim

         - name: Python Range-Doppler validation
           run: python3 scripts/plot_range_doppler.py build/range_doppler.csv

         - name: Upload validation artefacts
           uses: actions/upload-artifact@v4
           with:
             name: phase2-validation-${{ github.sha }}
             path: |
               build/range_doppler.csv
               range_doppler_map.png
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

Phase 2 Push Checklist
-----------------------

.. code-block:: bash

   # Switch to Phase 2 branch
   git checkout phase-2/signal-processing

   # Stage all Phase 2 files
   git add include/signal_processing.hpp include/cfar.hpp \
           src/signal_processing.cpp src/cfar.cpp \
           tests/test_range_doppler.cpp tests/test_cfar.cpp \
           scripts/plot_range_doppler.py \
           CMakeLists.txt \
           .github/workflows/phase2-ci.yml \
           docs/source/phase2/

   # Commit
   git commit -m "feat(phase2): Range-Doppler pipeline, CA-CFAR, FFTW3, CI"

   # Verify locally before pushing
   sudo apt install libfftw3-dev
   cmake -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
   cmake --build build --parallel
   ctest --test-dir build --output-on-failure -V
   ./build/radar_sim
   python3 scripts/plot_range_doppler.py build/range_doppler.csv

   # Push and open Pull Request
   git push -u origin phase-2/signal-processing

   # After green CI: merge and tag
   git checkout main
   git pull origin main
   git merge --no-ff phase-2/signal-processing
   git tag -a v0.2.0 -m "Phase 2 complete: Range-Doppler pipeline, CFAR"
   git push origin main --tags
