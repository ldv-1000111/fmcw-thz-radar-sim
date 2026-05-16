.. _phase2_embedded_opt:

Embedded Optimisation
======================

Running on automotive SoCs (NXP S32G, Renesas R-Car H3, Raspberry Pi 5)
requires careful real-time discipline.

Key Strategies
---------------

Pre-plan FFTW Transforms
^^^^^^^^^^^^^^^^^^^^^^^^^

Call ``fftwf_plan_*`` **once at startup** using ``FFTW_MEASURE`` or
``FFTW_PATIENT``. Store the plan and reuse it across all frames.
Planning once can improve per-frame execution time by 2–4× compared to
``FFTW_ESTIMATE``.

.. code-block:: cpp

   // At init time (called once):
   fftwf_plan plan = fftwf_plan_dft_1d(
       N, buf.data(), buf.data(), FFTW_FORWARD, FFTW_MEASURE);

   // In the frame loop (called every frame):
   fftwf_execute(plan);

Use ``float``, Not ``double``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The ``fftwf_*`` single-precision functions are 2–4× faster on ARM NEON SIMD
hardware versus the double-precision ``fftw_*`` variants. All Phase 1 and
Phase 2 buffers use ``std::complex<float>`` for this reason.

Moving-Average Alternative
^^^^^^^^^^^^^^^^^^^^^^^^^^^

For applications where frame rate must exceed 100 Hz, a sliding-window RMS
filter over slow-time provides a fast approximation to range profiling at
greatly reduced compute cost. See Marnach (2026) for a detailed
comparison of FFT versus filter-based approaches on 24 GHz FMCW hardware.

KISS FFT for Bare-Metal Targets
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For Cortex-M targets without an OS, KISS FFT provides a header-only,
zero-dependency C implementation as an alternative to FFTW3:

.. code-block:: bash

   # Add to your CMakeLists.txt via FetchContent
   FetchContent_Declare(
       kissfft
       GIT_REPOSITORY https://github.com/mborgerding/kissfft.git
       GIT_TAG        131.1.0
   )

Real-Time Scheduling
^^^^^^^^^^^^^^^^^^^^^

On the embedded Linux target, set ``SCHED_FIFO`` on the radar processing
thread to guarantee deterministic latency:

.. code-block:: bash

   # Requires root or CAP_SYS_NICE
   chrt -f 50 ./radar_sim
