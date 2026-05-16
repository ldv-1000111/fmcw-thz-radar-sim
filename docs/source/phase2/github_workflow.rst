.. _phase2_github_workflow:

Phase 2 GitHub Workflow
========================

Phase 2 extends the CI pipeline to install FFTW3 and run the new
``test_range_doppler.cpp`` and ``test_cfar.cpp`` tests.

.. code-block:: bash

   # Create branch
   git checkout -b phase-2/signal-processing

   # Add new files
   git add include/signal_processing.hpp include/cfar.hpp
   git add src/signal_processing.cpp src/cfar.cpp
   git add tests/test_range_doppler.cpp tests/test_cfar.cpp
   git add scripts/plot_range_doppler.py

   git commit -m "feat(phase2): Range-Doppler pipeline, CA-CFAR, FFTW3"
   git push -u origin phase-2/signal-processing

   # After green CI
   git checkout main
   git merge --no-ff phase-2/signal-processing
   git tag -a v0.2.0 -m "Phase 2 complete: Range-Doppler pipeline, CFAR"
   git push origin main --tags

Update ``.github/workflows/ci.yml`` to add FFTW3 installation:

.. code-block:: yaml

   - name: Install FFTW3
     run: sudo apt-get install -y libfftw3-dev
