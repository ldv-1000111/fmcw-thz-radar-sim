.. _milestones:

Tutorial Milestones
====================

.. rubric:: Milestone 1 — Single-Target IF Signal Validation (Phase 1)

**Goal:** Verify that ``generate_chirp_if()`` places the FFT peak at the
correct range bin.

#. Implement ``generate_chirp_if()`` using the code in
   :ref:`phase1_cpp_implementation`.
#. Run ``./build/radar_sim`` to generate ``if_signal.csv``.
#. Run ``python3 scripts/plot_if.py`` — output must print ``PASS``.
#. Compare: repeat with ``f0 = 77e9`` (mmWave) vs ``f0 = 300e9`` (THz)
   for the same 4 GHz bandwidth. Note the identical range resolution
   (it depends only on bandwidth, not carrier frequency).
#. All 7 Catch2 tests pass. Tag ``v0.1.0``.

.. note::

   Expected range bin for a 50 m target:
   ``k = round( (B/Tc) * (2*R/c) * Ns / fs ) = 133``
   for the default parameters in ``main.cpp``.

.. rubric:: Milestone 2 — 2D Range-Doppler Map (Phase 2)

**Goal:** Build the complete data cube pipeline and verify multi-target
detection with micro-Doppler signatures.

#. Extend the simulation to :math:`N_c = 256` chirps per frame.
#. Implement ``compute_range_doppler()`` using FFTW3.
#. Simulate three targets:

   * (R = 20 m, v = +5 m/s)
   * (R = 50 m, v = 0 m/s) — add micro-Doppler: A = 0.2 mm, f = 200 Hz
   * (R = 80 m, v = −8 m/s)

#. Verify three peaks at correct range/velocity bins.
#. Verify that the second target shows sidebands at ±200 Hz.
#. Run CA-CFAR and print detected target reports to stdout.
#. All Phase 2 Catch2 tests pass. Tag ``v0.2.0``.

.. rubric:: Milestone 3 — Yocto Cross-Compilation (Phase 3)

**Goal:** Deploy the simulator to an embedded Linux target and profile it.

#. Build for QEMU x86-64 and confirm ``radar_sim`` runs in emulation.
#. Cross-compile for Raspberry Pi 5 (``MACHINE = "raspberrypi5"``).
#. Flash the SD card image and run the simulator on hardware.
#. Profile with ``perf stat`` — note cache miss rates and IPC.
#. Parallelise the Doppler FFT loop with ``std::thread`` across range bins.
#. Measure frames-per-second improvement on the multi-core target.
#. Tag ``v0.3.0``.
