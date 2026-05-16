.. _phase2_data_cube:

Data Cube Architecture
=======================

A frame of :math:`N_c` chirps stacked in slow-time forms the **radar data
cube**. Each axis maps to a physical measurement after FFT processing.

.. list-table::
   :header-rows: 1
   :widths: 22 22 20 36

   * - Axis
     - Dimension
     - After FFT
     - Encodes
   * - Axis 0 — Fast-time
     - :math:`N_s` samples/chirp
     - Range FFT
     - Target distance
   * - Axis 1 — Slow-time
     - :math:`N_c` chirps/frame
     - Doppler FFT
     - Radial velocity
   * - Axis 2 — Spatial
     - :math:`N_\text{rx}` antennas
     - Angle FFT
     - Azimuth / elevation

Memory Layout
--------------

The data cube is stored as a 2D C++ vector ``cube[chirp][sample]`` using
row-major ordering for cache-friendly sequential access during the Range FFT:

.. code-block:: cpp

   // data_cube[num_chirps][num_samples]
   std::vector<std::vector<std::complex<float>>> data_cube(
       num_chirps,
       std::vector<std::complex<float>>(num_samples)
   );

   // Fill: one chirp at a time
   for (int c = 0; c < num_chirps; ++c)
       generate_chirp_if(p, tgt, c, data_cube[c]);

See Liu et al. (2022) for a discussion of optimal CPU/GPU memory layouts
for large MIMO data cubes.
