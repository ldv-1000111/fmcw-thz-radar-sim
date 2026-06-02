.. _phase2_data_cube:

Data Cube Architecture
=======================

A frame of :math:`N_c` chirps stacked in slow-time forms the **radar data
cube** — a three-dimensional array of complex samples that contains all
the information needed to determine the range and velocity of every target
in the scene. Each axis maps to a physical measurement after FFT processing.

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

.. note::

   Phase 2 implements Axes 0 and 1 only. The spatial axis (Axis 2) is
   reserved for a future MIMO extension.

----

Theory — Fast-Time and Slow-Time
----------------------------------

Understanding the data cube requires understanding two distinct time axes
that operate at very different scales and carry completely different
physical information.

Fast-Time
^^^^^^^^^^

Fast-time is the time axis **within a single chirp**. For each chirp the
radar transmits a frequency sweep and samples the IF beat signal at the
ADC sampling rate ``fs``. With ``fs = 50 MHz`` and ``Tc = 100 µs`` you
collect 5000 samples per chirp, each 20 ns apart.

The physical meaning of fast-time is **round-trip propagation delay**.
A target at range ``R`` produces a delay ``τ = 2R/c``. In the FMCW
deramp mixer this delay becomes a beat frequency:

.. math::

   f_{\text{beat}} = \frac{B}{T_c} \cdot \tau = \frac{B}{T_c} \cdot \frac{2R}{c}

The FFT over the fast-time samples converts this tone into a peak at
the range bin corresponding to ``R``. **Fast-time is a measurement of
distance** — range encoded as frequency.

Slow-Time
^^^^^^^^^^

Slow-time is the time axis **across multiple chirps**. Each chirp
provides one slow-time sample. With ``NC = 256`` chirps repeating every
``Tc = 100 µs``, the slow-time axis spans 25.6 ms at a sampling rate of
``1/Tc = 10 kHz``.

The physical meaning of slow-time is **target displacement between
chirps**. A target moving at velocity ``v`` shifts its range by
``v·Tc`` between consecutive chirps, changing the initial phase of the
IF signal by:

.. math::

   \Delta\phi = 2\pi f_0 \cdot \frac{2v \cdot T_c}{c}
              = 2\pi \cdot \frac{2 f_0 v}{c} \cdot T_c

This is a **linear phase ramp** across the slow-time axis. The FFT
over the slow-time samples converts this ramp into a peak at the
Doppler bin corresponding to ``v``. **Slow-time is a measurement of
velocity** — velocity encoded as phase rotation rate.

Why They Are Independent
^^^^^^^^^^^^^^^^^^^^^^^^^

Within one chirp (fast-time), the target moves only ``v·Tc`` during
the entire chirp — for ``v = 1 m/s`` and ``Tc = 100 µs`` that is
0.1 mm, negligible relative to the range resolution of 3.75 cm.
Fast-time sees range only.

Across chirps (slow-time), the beat frequency shifts by less than one
range bin per chirp for any practical velocity. Slow-time sees velocity
only through the phase progression.

This separation is what makes the 2D FFT processing valid. If targets
moved fast enough to shift range bins between chirps the two axes would
be coupled — this is the **range migration** problem that SAR must
solve but short-range FMCW can ignore.

.. list-table:: Fast-time vs slow-time
   :header-rows: 1
   :widths: 25 37 38

   * - Property
     - Fast-time
     - Slow-time
   * - Samples
     - ``NS`` samples within one chirp
     - ``NC`` chirps, one sample each
   * - Sampling rate
     - ``fs = 50 MHz`` (20 ns between samples)
     - ``1/Tc = 10 kHz`` (100 µs between samples)
   * - Duration
     - ``Tc = 100 µs``
     - ``NC·Tc = 25.6 ms``
   * - Physical measurement
     - Round-trip propagation delay
     - Inter-chirp phase evolution
   * - FFT reveals
     - **Range**
     - **Velocity**
   * - Resolution
     - ``δR = c/(2B) = 3.75 cm``
     - ``δv = c/(2f₀·NC·Tc) = 0.0195 m/s``

----

Data Cube Structure
--------------------

Stacking ``NC`` chirps produces the cube visualised below:

.. code-block:: text

                   range bins (fast-time FFT →)
                 ┌─────────────────────────────┐
   chirp 0 (c=0) │  ·  ·  ·  ●  ·  ·  ·  ·  ·  │
   chirp 1 (c=1) │  ·  ·  ·  ●  ·  ·  ·  ·  ·  │  ← same range bin,
   chirp 2 (c=2) │  ·  ·  ·  ●  ·  ·  ·  ·  ·  │     phase rotating
      ...        │  ·  ·  ·  ●  ·  ·  ·  ·  ·  │     each row
   chirp NC-1    │  ·  ·  ·  ●  ·  ·  ·  ·  ·  │
                 └─────────────────────────────┘
                             ↑
                   take this column and apply
                   slow-time FFT → velocity

Each **row** is one chirp. The range FFT runs horizontally and places
the target at the correct range bin. Reading **down a column** at that
range bin, the complex value rotates in phase from row to row at a rate
set by the target's velocity. The Doppler FFT runs vertically and
converts the rotation rate into a velocity bin.

----

Memory Layout
--------------

The data cube is stored as a 2D C++ vector ``cube[chirp][sample]`` using
row-major ordering for cache-friendly sequential access during the Range
FFT:

.. code-block:: cpp

   // cube[num_chirps][num_samples]
   std::vector<std::vector<std::complex<float>>> cube(
       num_chirps,
       std::vector<std::complex<float>>(num_samples, {0.0f, 0.0f})
   );

   // Fill: one chirp at a time
   for (int c = 0; c < num_chirps; ++c)
       generate_chirp_if(p, tgt, c, cube[c]);

See Liu et al. (2022) :cite:`liu2022` for a discussion of optimal CPU/GPU
memory layouts for large MIMO data cubes.

----

Building the Data Cube — Multiple Targets
------------------------------------------

Phase 2 superpositions three targets into the same cube. The IF signal is
linear — responses from multiple targets add directly:

.. code-block:: cpp

   // Amplitude scale factors — moving targets given higher amplitude
   // to remain visible above range FFT sidelobes of the stationary target
   const std::vector<float> amp_scale = { 1.0f, 10.0f, 10.0f };

   std::vector<std::complex<float>> buf(p.num_samples);
   for (int t = 0; t < static_cast<int>(targets.size()); ++t) {
       for (int c = 0; c < p.num_chirps; ++c) {
           generate_chirp_if(p, targets[t], c, buf);
           for (int s = 0; s < p.num_samples; ++s)
               cube[c][s] += buf[s] * amp_scale[t];
       }
   }

.. note::

   The amplitude scaling is a practical necessity for multi-target
   simulation. A dominant stationary target produces range FFT sidelobes
   that can bury weaker moving targets at nearby range bins. In a real
   system the targets have different radar cross-sections (RCS) that
   play the same role. See :ref:`phase2_lessons_learned` for the full
   quantitative analysis.
