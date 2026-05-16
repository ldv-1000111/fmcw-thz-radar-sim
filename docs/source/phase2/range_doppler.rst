.. _phase2_range_doppler:

Range-Doppler Pipeline (FFTW3)
================================

FFTW3 (*Fastest Fourier Transform in the West*) is the de-facto open-source C
library for high-performance FFTs. It generates architecture-specific SIMD code
at plan time, exploiting ARM NEON and x86 AVX2 automatically.

``include/signal_processing.hpp``
-----------------------------------

.. code-block:: cpp
   :caption: include/signal_processing.hpp
   :linenos:

   #pragma once
   #include <complex>
   #include <vector>

   // Compute 2D Range-Doppler magnitude map from data cube.
   // cube[chirp][sample] → rd_map[doppler_bin][range_bin]
   void compute_range_doppler(
       const std::vector<std::vector<std::complex<float>>>& cube,
       int num_chirps,
       int num_samples,
       std::vector<std::vector<float>>& rd_map   // output: magnitude
   );

``src/signal_processing.cpp``
-------------------------------

.. code-block:: cpp
   :caption: src/signal_processing.cpp
   :linenos:

   #include "signal_processing.hpp"
   #include <fftw3.h>
   #include <cmath>

   void compute_range_doppler(
       const std::vector<std::vector<std::complex<float>>>& cube,
       int num_chirps, int num_samples,
       std::vector<std::vector<float>>& rd_map)
   {
       // ── Step 1: Range FFT along fast-time axis ───────────────
       std::vector<fftwf_complex> rbuf(num_samples);
       // Use FFTW_MEASURE in production; FFTW_ESTIMATE is faster to plan
       fftwf_plan rplan = fftwf_plan_dft_1d(
           num_samples, rbuf.data(), rbuf.data(),
           FFTW_FORWARD, FFTW_ESTIMATE);

       std::vector<std::vector<std::complex<float>>> rout(
           num_chirps, std::vector<std::complex<float>>(num_samples));

       for (int c = 0; c < num_chirps; ++c) {
           for (int s = 0; s < num_samples; ++s) {
               rbuf[s][0] = cube[c][s].real();
               rbuf[s][1] = cube[c][s].imag();
           }
           fftwf_execute(rplan);
           for (int s = 0; s < num_samples; ++s)
               rout[c][s] = {rbuf[s][0], rbuf[s][1]};
       }
       fftwf_destroy_plan(rplan);

       // ── Step 2: Doppler FFT along slow-time axis ─────────────
       std::vector<fftwf_complex> dbuf(num_chirps);
       fftwf_plan dplan = fftwf_plan_dft_1d(
           num_chirps, dbuf.data(), dbuf.data(),
           FFTW_FORWARD, FFTW_ESTIMATE);

       rd_map.assign(num_chirps,
           std::vector<float>(num_samples, 0.0f));

       for (int s = 0; s < num_samples; ++s) {
           for (int c = 0; c < num_chirps; ++c) {
               dbuf[c][0] = rout[c][s].real();
               dbuf[c][1] = rout[c][s].imag();
           }
           fftwf_execute(dplan);
           for (int c = 0; c < num_chirps; ++c)
               rd_map[c][s] = std::hypot(dbuf[c][0], dbuf[c][1]);
       }
       fftwf_destroy_plan(dplan);
   }

.. tip::

   Plan FFTW transforms **once at startup** using ``FFTW_MEASURE`` or
   ``FFTW_PATIENT``. Store the plan and reuse across all frames.
   Planning once can improve per-frame execution time by 2–4×.
