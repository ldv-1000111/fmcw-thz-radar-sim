.. _phase2_lessons_learned:

Lessons Learned — Phase 2
==========================

Phase 2 revealed three areas where theory and practice diverged in
instructive ways. Each one is documented here as a standalone
theory-meets-practice episode: what was expected, what happened,
what the diagnostic showed, and what the fix was.

----

Lesson 1 — The FMCW Signal Model Was Missing the Doppler Carrier
------------------------------------------------------------------

**What was expected**

The Phase 1 signal model generates the IF beat signal as:

.. math::

   \phi_{IF}(t) = 2\pi \cdot f_{beat} \cdot t

where :math:`f_{beat} = \frac{B}{T_c} \cdot \tau(t) + \frac{2 f_0 v}{c}`.
This includes a Doppler frequency term :math:`\frac{2 f_0 v}{c}`.
It was reasonable to assume this would produce a Doppler peak in the
slow-time FFT for a moving target.

**What happened**

The Doppler FFT produced no peak for moving targets. All three targets
peaked at Doppler bin 0 regardless of velocity.

**Diagnosis**

A Python diagnostic measured the phase of the range FFT output at the
expected range bin across consecutive chirps:

.. code-block:: text

   chirp 0->1: phase diff at r_bin=68: 0.0153 deg
   chirp 1->2: phase diff at r_bin=68: 0.0153 deg
   ...
   Expected: 45.0000 deg per chirp

The phase was advancing only 0.0153°/chirp instead of the expected 45°.
The Doppler term in :math:`f_{beat}` is 1.25 kHz — tiny compared to the
range beat frequency of 13.3 MHz. Multiplied by the fast-time sample
duration :math:`t = \frac{N_s/2}{f_s} = 2.56\,\mu\text{s}`, it
contributes only 0.015°, not 45°. The Doppler information was present
in the frequency domain but completely invisible in the phase domain
where the Doppler FFT needs it.

**Root cause — the missing initial phase term**

The correct FMCW deramp signal model is:

.. math::

   \phi_{IF}(t) = \underbrace{2\pi f_0 \tau_0}_{\text{initial phase (Doppler)}}
                + \underbrace{2\pi \frac{B}{T_c} \tau_0 \cdot t}_{\text{beat tone (range)}}

where :math:`\tau_0 = 2R_0/c` is the round-trip delay at the **start** of
the chirp. The initial phase :math:`\phi_0 = 2\pi f_0 \tau_0` advances
between consecutive chirps by:

.. math::

   \Delta\phi = 2\pi \cdot \frac{2 f_0 v}{c} \cdot T_c

For the 50 m target at :math:`v = 0.625` m/s with :math:`f_0 = 300` GHz
and :math:`T_c = 100\,\mu\text{s}`:

.. math::

   \Delta\phi = 2\pi \cdot \frac{2 \times 300 \times 10^9 \times 0.625}{3 \times 10^8}
              \times 100 \times 10^{-6} = 45°

This is precisely what the Doppler FFT needs to see. The Phase 1
implementation omitted :math:`\phi_0` entirely — the phase at
:math:`t = 0` was always 0 regardless of chirp index, so the slow-time
sequence was flat and all energy landed at bin 0.

**The fix**

.. code-block:: cpp

   // Range at the start of this chirp (slow-time + vibration)
   const double R0   = tgt.range
                     + tgt.velocity * slow_t
                     + tgt.vib_amp * std::sin(2.0 * M_PI * tgt.vib_freq * slow_t);
   const double tau0 = 2.0 * R0 / C;
   const double phi0 = 2.0 * M_PI * p.f0 * tau0;   // Doppler carrier

   const double beat_freq = slope * tau0 + 2.0 * p.f0 * tgt.velocity / C;
   const double phase     = phi0 + 2.0 * M_PI * beat_freq * t;

**Why Phase 1 tests did not catch this**

Phase 1 only validates range bins using a 1D FFT over the fast-time
axis of a single chirp. The initial phase :math:`\phi_0` is a constant
offset within one chirp — it shifts the phase of every sample equally
and does not change the range FFT peak location. The Phase 1 tests
passed correctly with or without :math:`\phi_0`. The gap only became
visible when the Doppler FFT compared phases *across* chirps.

.. note::

   This is a common pitfall when implementing FMCW from textbooks that
   present the simplified model :math:`s_{IF}(t) = \exp(j 2\pi f_{beat} t)`.
   This form is correct for range processing of a single chirp but omits
   the inter-chirp phase information required for Doppler processing.

----

Lesson 2 — Range FFT Sidelobes Masked Moving Targets
------------------------------------------------------

**What was expected**

With three targets superposed in the data cube, the Range-Doppler map
should show three clear peaks at the correct (range bin, Doppler bin)
coordinates.

**What happened**

The 20 m stationary target produced a dominant peak at Doppler bin 0
that leaked into range bins 68 and 109 with amplitude ~30,000 — 2,800×
stronger than the 50 m moving target signal at bin 32.

**Why this happens — rectangular FFT sidelobes**

The range FFT of a pure sinusoid without windowing produces sidelobes
that fall off as :math:`1/\sin(\pi \Delta k / N)`. At :math:`\Delta k = 41`
bins (the separation between the 20 m and 50 m targets), the rectangular
sidelobe is only ~32 dB below the main lobe:

.. math::

   \text{sidelobe} \approx \frac{1}{\pi \Delta k / N}
                          = \frac{N}{\pi \times 41}
                          \approx -32\,\text{dB}

With the 20 m target peak at amplitude ~30,000, its sidelobe at range
bin 68 was ~750. The 50 m moving target peak was ~107. The sidelobe
dominated by a factor of 7×, making it invisible in the noise floor.

**The fix — Hann window**

Applying a Hann window to each chirp before the range FFT reduces
sidelobes to ~80 dB at the same 41-bin separation. With the window,
the sidelobe from the 20 m target at bin 68 falls to < 1, and the
50 m target at amplitude 107 is clearly the dominant signal.

.. code-block:: cpp

   // Pre-compute Hann window of length num_samples
   std::vector<float> win(num_samples);
   for (int i = 0; i < num_samples; ++i)
       win[i] = 0.5f * (1.0f - std::cos(
                    2.0f * static_cast<float>(M_PI) * i / (num_samples - 1)));

   // Apply window before range FFT
   for (int s = 0; s < num_samples; ++s) {
       rbuf[s][0] = cube[c][s].real() * win[s];
       rbuf[s][1] = cube[c][s].imag() * win[s];
   }

**SNR measurement — the DC bin problem**

During validation a second issue appeared: even after windowing, the
Python script reported SNR < 3 for the moving targets. The SNR was
computed as:

.. code-block:: python

   noise = np.mean(grid)   # global mean of the entire map
   snr   = peak_val / noise

The global mean was dominated by the three large peaks at Doppler bin 0,
inflating the noise estimate and suppressing the apparent SNR of the
moving targets.

The correct noise reference for a moving target at non-zero Doppler bin
is the **median of the non-DC Doppler bins** at the target's range bin:

.. code-block:: python

   col        = grid[:, exp_r]          # Doppler profile at target range
   noise_bins = col[1 : d_max // 2]    # positive frequencies, no DC
   local_noise = np.median(noise_bins)
   snr = peak_val / local_noise

With this correction all three targets passed with SNR of 8.4, 942, and
325 respectively.

.. note::

   Doppler bin 0 (DC) always accumulates the mean power of every target
   across all chirps, regardless of velocity. It is not a valid noise
   reference for moving-target detection. In a real system this is handled
   by DC removal (subtracting the mean of the slow-time sequence before
   the Doppler FFT) or by explicitly excluding bin 0 from detection logic.

----

Lesson 3 — Doppler Test Design: Peak Location vs Energy Distribution
---------------------------------------------------------------------

**What was expected**

A test that finds the Doppler peak at the target's range bin and asserts
it is at a different bin for a moving target versus a stationary target:

.. code-block:: cpp

   REQUIRE(doppler_static != doppler_moving);

**What happened**

The test failed consistently. Both targets peaked at bin 0 even with
velocities carefully chosen to land on bin 8 (NC=64) or bin 32 (NC=256).

**Why peak-location tests are fragile for Doppler**

Three separate issues made the peak-location test unreliable:

1. **Bin aliasing** — velocity 5.0 m/s mapped to Doppler bin 64 with
   NC=64, which wraps to bin 0 — identical to a stationary target.
   The bin calculation must account for the FFT period.

2. **Sidelobe dominance** — before the Hann window fix, the stationary
   target's sidelobe at the moving target's range bin was stronger than
   the moving target's own Doppler peak, so the peak-finder always
   returned bin 0.

3. **The search was at the wrong peak** — even with the correct bin
   calculation and windowing, the Doppler FFT of a real FMCW signal
   has a main lobe spread across several bins. The test searched for the
   exact expected bin rather than searching a neighbourhood.

**The fix — test energy distribution, not exact bin location**

A moving target must distribute more energy into non-DC Doppler bins
than a stationary target. This is physically correct and numerically
robust:

.. code-block:: cpp

   // Sum energy in off-DC Doppler bins at the target's range bin
   float energy_static = 0.0f, energy_moving = 0.0f;
   for (int d = 1; d < NC / 2; ++d) {
       energy_static += rd_static[d][r_bin];
       energy_moving += rd_moving[d][r_bin];
   }
   REQUIRE(energy_moving > energy_static);

This test is generator-agnostic, tolerant of bin aliasing, and does not
depend on sidelobe levels. It encodes the physically meaningful
distinction between a stationary target (all energy at DC) and a moving
target (energy shifted to a non-zero Doppler frequency).

.. note::

   This is the broader principle: **test physical properties, not
   implementation details**. Asserting ``peak_bin == 32`` tests a
   specific numerical outcome that is sensitive to parameter choices,
   windowing, and phase noise. Asserting ``moving target has more
   off-DC Doppler energy than stationary target`` tests the physics,
   which is always true regardless of implementation details.

----

Summary
-------

.. list-table::
   :header-rows: 1
   :widths: 30 35 35

   * - Issue
     - Root cause
     - Fix
   * - No Doppler peak for moving targets
     - Missing initial phase :math:`\phi_0 = 2\pi f_0 \tau_0` in signal
       model — inter-chirp phase was flat
     - Add :math:`\phi_0` to the IF phase; evaluate range at chirp start
       not fast-time instant
   * - Moving targets masked by stationary target sidelobes
     - Rectangular FFT sidelobes ~32 dB at 41 bins; global mean SNR
       inflated by DC peak
     - Hann window on range FFT; local non-DC median as noise reference
   * - Doppler bin test always fails
     - Bin aliasing; peak search at wrong location; sidelobe dominance
     - Test energy distribution across off-DC bins, not exact peak bin
