.. _phase1_fmcw_theory:

FMCW Signal Theory
===================

This page derives the equations you will implement in
:ref:`phase1_cpp_implementation`. Read it before touching the keyboard.

The FMCW Chirp
---------------

An FMCW radar continuously transmits a linearly swept frequency ramp called a
**chirp**. The instantaneous phase of a single chirp is:

.. math::

   \phi(t) = 2\pi \left[ f_0 t + \frac{B}{2 T_c} t^2 \right]

where:

* :math:`f_0` — start frequency (Hz), e.g. 300 × 10⁹ for a 300 GHz THz radar
* :math:`B` — sweep bandwidth (Hz), governs range resolution
* :math:`T_c` — chirp duration (s)
* :math:`t` — fast-time sample index within the chirp

.. warning::

   Do **not** simulate the raw GHz or THz carrier signal directly. At 300 GHz,
   satisfying the Nyquist criterion requires a sampling rate exceeding 600 GHz —
   computationally catastrophic. Simulate the **IF beat signal** instead, as
   emphasised by Schasler et al. (2021).

The IF Beat Signal
-------------------

When the reflected wave returns from a target at range :math:`R` moving at
radial velocity :math:`v`, it arrives after a round-trip delay:

.. math::

   \tau(t) = \frac{2 \bigl(R + v\,t\bigr)}{c}

Mixing the received signal with the currently transmitted chirp produces the
**Intermediate Frequency (IF) beat signal**:

.. math::

   f_\text{beat} = \frac{B}{T_c}\,\tau + \frac{2 f_0 v}{c}

The IF signal in analytic (complex exponential) form is:

.. math::

   s_\text{IF}(t) = e^{\,j\, 2\pi f_\text{beat}\, t}

Key Performance Metrics
------------------------

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Metric
     - Formula
   * - Range resolution
     - :math:`\Delta r = \dfrac{c}{2B}`
   * - Max unambiguous range
     - :math:`R_\text{max} = \dfrac{f_s \cdot c}{2\,(B/T_c)}`
   * - Doppler (velocity) resolution
     - :math:`\Delta v = \dfrac{\lambda}{2\,N_c\,T_c}`
   * - Max unambiguous velocity
     - :math:`v_\text{max} = \dfrac{\lambda}{4\,T_c}`
   * - Range bin index
     - :math:`k_r = \operatorname{round}\!\left(\dfrac{f_\text{beat}\,N_s}{f_s}\right)`

Example — 300 GHz THz System
------------------------------

.. list-table::
   :header-rows: 1
   :widths: 30 20 50

   * - Parameter
     - Value
     - Notes
   * - :math:`f_0`
     - 300 GHz
     - :math:`\lambda = 1\,\text{mm}`
   * - :math:`B`
     - 4 GHz
     - :math:`\Delta r = 3.75\,\text{cm}`
   * - :math:`T_c`
     - 100 µs
     -
   * - :math:`N_c`
     - 256 chirps
     - :math:`\Delta v = 1.95\,\text{mm/s}`
   * - :math:`f_s`
     - 50 MHz
     - :math:`N_s = 5000` samples/chirp
   * - Frame rate
     - ≈ 39 Hz
     - :math:`1 / (N_c \cdot T_c)`

Terahertz-Specific Effects
---------------------------

Micro-Doppler Signatures
^^^^^^^^^^^^^^^^^^^^^^^^^

At THz wavelengths (~1 mm), sub-millimetre structural vibrations produce
**large, distinct phase modulations** in slow-time. A target undergoing
harmonic vibration has an effective displacement:

.. math::

   R(t) = R_0 + v\,t + A_\text{vib}\,\sin(2\pi f_\text{vib}\,t)

Substituting into :math:`\tau(t)` causes sinusoidal phase modulation across
chirps, producing characteristic sidebands at :math:`\pm f_\text{vib}` in the
Doppler spectrum. This is the **micro-Doppler signature** unique to THz sensing.

Engine idle (~200 Hz), drone propeller rotation, and pedestrian limb
micro-motion all produce identifiable sideband patterns — a key THz advantage
over 77 GHz mmWave where the wavelength is too long to resolve sub-mm motion.
See Han (2026) for measured drone micro-Doppler datasets.

Atmospheric Absorption
^^^^^^^^^^^^^^^^^^^^^^

Water vapour, oxygen, and CO₂ absorb THz energy at resonance bands. Path loss
must account for atmospheric attenuation beyond standard free-space (Friis)
propagation, especially for range bins beyond 50 m. Use ITU-R P.676 tables for
band-specific absorption coefficients at your operating frequency.

Material Fingerprinting
^^^^^^^^^^^^^^^^^^^^^^^

THz waves stimulate rotational and vibrational modes in polymers, organics, and
composites, producing unique spectral absorption signatures. This enables
**material classification** — distinguishing clothing from metal panels, or
identifying concealed substances — beyond simple reflectivity-based detection.
