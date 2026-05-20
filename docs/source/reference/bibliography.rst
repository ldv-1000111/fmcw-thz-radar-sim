.. _bibliography:

Bibliography
============

.. note::

   This bibliography is divided into six sections.  The first three —
   **Foundational FMCW Radar Theory**, **Micro-Doppler Theory**, and
   **THz Propagation and Atmospheric Effects** — are the primary literature
   sources for the signal model and equations implemented in
   ``src/fmcw_generator.cpp`` and declared in ``include/fmcw_generator.hpp``.
   Every equation in :ref:`fmcw_theory` traces directly to one or more of
   these entries.

   The remaining three sections — **Core Radar Simulation**, **Embedded
   Signal Processing**, and **Terahertz Technology — Published Research** —
   cover simulation frameworks, embedded DSP references, and industry/hardware
   sources that contextualise the application layer.  They do *not* contain
   the derivations the code encodes.

   The Teradar commercial white papers (ADAS, Defense, Healthcare, Security)
   are cited for validated performance metrics and application context only;
   the foundational physics is traceable exclusively to the peer-reviewed
   sources in sections A–C below.


.. rubric:: A — Foundational FMCW Radar Theory

.. [stove1992]
   Stove, A. G. (1992). Linear FMCW radar techniques.
   *IEE Proceedings F — Radar and Signal Processing*, 139(5), 343–350.
   https://doi.org/10.1049/ip-f-2.1992.0048
   *(Canonical derivation of the IF beat-frequency equation
   f_beat = (B/Tc)·τ + 2·f0·v/c and the chirp-slope parameter μ = B/Tc.
   Primary source for the chirp phase model, round-trip delay, and beat
   frequency — equations 1, 2, and 3 in* :ref:`fmcw_theory` *.)*

.. [richards2010]
   Richards, M. A., Scheer, J. A., & Holm, W. A. (Eds.). (2010).
   *Principles of Modern Radar: Basic Principles*. SciTech Publishing.
   ISBN 978-1-891121-52-4.
   *(Comprehensive reference for range resolution Δr = c/2B, range-Doppler
   map construction, velocity resolution, and range-bin index mapping.
   Primary source for equations 1, 3, 6, 7, and 10 in* :ref:`fmcw_theory` *.)*

.. [skolnik2001]
   Skolnik, M. I. (2001). *Introduction to Radar Systems* (3rd ed.).
   McGraw-Hill. ISBN 978-0-07-290980-9.
   *(Standard reference for the radar range equation, SNR, receiver noise
   figure, Doppler frequency derivation, and maximum unambiguous range.
   Primary source for equations 2, 8, and 12 in* :ref:`fmcw_theory` *.)*

.. [rohling2001]
   Rohling, H., & Meinecke, M. M. (2001). Waveform design principles for
   automotive radar systems. *Proceedings of the 2001 IEEE Radar Conference*,
   1–7.
   https://doi.org/10.1109/NRC.2001.922968
   *(Derivation of velocity resolution Δv = λ/(2·Nc·Tc) and maximum
   unambiguous velocity v_max = λ/(4·Tc) for automotive FMCW systems.
   Primary source for equations 3, 4, 7, and 9 in* :ref:`fmcw_theory` *.)*

.. [meta2007]
   Meta, A., Hoogeboom, P., & Ligthart, L. P. (2007). Signal processing for
   FMCW SAR. *IEEE Transactions on Geoscience and Remote Sensing*, 45(11),
   3519–3532.
   https://doi.org/10.1109/TGRS.2007.903703
   *(Rigorous treatment of the analytic complex-exponential IF signal model
   and range-bin index mapping k_r = round(f_beat·Ns/fs).  Primary source
   for equations 4 and 10 in* :ref:`fmcw_theory` *. Also justifies — together
   with* :cite:`schasler2021` *— the design decision to simulate the IF beat
   signal rather than the GHz/THz carrier directly.)*


.. rubric:: B — Micro-Doppler Theory

.. [chen2011]
   Chen, V. C. (2011). *The Micro-Doppler Effect in Radar*. Artech House.
   ISBN 978-1-60807-240-8.
   *(Definitive monograph on micro-Doppler signatures. Primary source for
   the sinusoidal vibration model R(t) = R0 + v·t + A_vib·sin(2π·f_vib·t),
   limb-motion cadence parameters (pedestrian ~2.5 Hz, drone rotor ~80 Hz,
   cyclist ~1.5 Hz), and STFT spectrogram representation used in the
   micro-Doppler visualisation. Primary source for equation 5 in*
   :ref:`fmcw_theory` *.)*

.. [chen2006]
   Chen, V. C., Li, F., Ho, S.-S., & Wechsler, H. (2006). Micro-Doppler
   effect in radar: phenomenon, model, and simulation study.
   *IEEE Transactions on Aerospace and Electronic Systems*, 42(1), 2–21.
   https://doi.org/10.1109/TAES.2006.1603402
   *(Primary peer-reviewed paper establishing the sinusoidal micro-Doppler
   phase-modulation model implemented in ``generate_chirp_if()``.
   Demonstrates that at THz wavelengths (~1 mm) sub-millimetre vibrations
   produce sideband amplitudes comparable to the main target return — an
   effect negligible at 77 GHz mmWave (λ = 3.9 mm).  Primary source for
   equation 5 in* :ref:`fmcw_theory` *.)*


.. rubric:: C — THz Propagation and Atmospheric Effects

.. [federici2005]
   Federici, J. F., Schulkin, B., Huang, F., Gary, D., Barat, R.,
   Oliveira, F., & Zimdars, D. (2005). THz imaging and sensing for security
   applications — explosives, weapons and drugs.
   *Semiconductor Science and Technology*, 20(7), S266–S280.
   https://doi.org/10.1088/0268-1242/20/7/018
   *(Atmospheric window positions, water-vapour absorption bands, and
   material fingerprinting via THz spectroscopy. Provides the physical
   basis for the atmospheric attenuation term L_atm = 2·α·R in equation 11
   of* :ref:`fmcw_theory` *. Also the primary academic source behind the
   material-fingerprinting claims in the Teradar ADAS and security
   white papers.)*

.. [kleine2011]
   Kleine-Ostmann, T., & Nagatsuma, T. (2011). A review of terahertz
   communications research. *Journal of Infrared, Millimeter, and Terahertz
   Waves*, 32(2), 143–171.
   https://doi.org/10.1007/s10943-010-9440-2
   *(Atmospheric absorption windows at 183 GHz, 325 GHz, and 557 GHz;
   propagation loss coefficients across the 0.1–10 THz band. Primary source
   for equation 11 and the logarithmic frequency-band classifications
   (mm-wave / sub-THz / THz) used in the simulator GUI and the sensor
   landscape page.)*


.. rubric:: D — Core Radar Simulation

.. [liu2022]
   Liu, G., Yang, W., Li, P., Qin, G., Cai, J., Wang, Y., Wang, S., Yue, N.,
   & Huang, D. (2022). MIMO Radar Parallel Simulation System Based on CPU/GPU
   Architecture. *Sensors*, 22(1), 396.
   https://doi.org/10.3390/s22010396
   *(Open Access, PMC full text — cited 21 times)*

.. [schasler2021]
   Schasler, C., Hoffmann, M., Braunig, J., Ullmann, I., Ebelt, R., &
   Vossiek, M. (2021). A Realistic Radar Ray Tracing Simulator for Large
   MIMO-Arrays in Automotive Environments. *IEEE Journal of Microwaves*,
   1(4), 962–974.
   https://doi.org/10.1109/JMW.2021.3104722
   *(IEEE Xplore — cited 75 times. Also justifies IF-domain simulation —
   avoiding direct carrier-frequency sampling — shared with* :cite:`meta2007` *.)*

.. [martin2022]
   Martin, M. Y., Winberg, S. L., Gaffar, M. Y. A., & Macleod, D. (2022).
   The Design and Implementation of a Ray-tracing Algorithm for Signal-level
   Pulsed Radar Simulation Using the NVIDIA® OptiX™ Engine.
   *Journal of Communications*, 17(9), 761–768.
   https://doi.org/10.12720/jcm.17.9.761-768
   *(GitHub: RTS module — cited 14 times)*

.. [mercuri2022]
   Mercuri, M., Russo, P., Glassee, M., Castro, I. D., De Greef, E.,
   Rykunov, M., Bauduin, M., Bourdoux, A., Ocket, I., Crupi, F., & Torfs, T.
   (2022). Automatic radar-based 2-D localization exploiting vital signs
   signatures. *Scientific Reports*, 12, Article 11671.
   https://doi.org/10.1038/s41598-022-11671-1
   *(Open Access — cited 31 times)*

.. [srivastava2025]
   Srivastava, S., Li, J., Mishra, P., Bansal, K., & Bharadia, D. (2025).
   A Realistic Radar Simulator for End-to-End Autonomous Driving in CARLA
   *(C-Shenron)*. *IEEE VTC 2025*.
   https://doi.org/10.1109/VTC2025-Fall65116.2025.11310463
   *(Open Access, GitHub)*

.. [han2026]
   Han, S. K. (2026). A Time-Synchronized Multi-Sensor drone dataset acquired
   from multiple radars and RF receiver. *PMC*.
   *(Measured drone micro-Doppler datasets. Validates the target cadence
   parameters in equation 5 — see* :cite:`chen2006` *and* :cite:`chen2011` *.)*


.. rubric:: E — Embedded Signal Processing

.. [marnach2026]
   Marnach, A. (2026). Comprehensive signal processing approaches for
   non-contact heartbeat detection using 24 GHz FMCW radar.
   *ARS — Volumes*, 23, 127.
   *(Moving-Average and Moving-RMS embedded filter reference for Phase 2
   CA-CFAR and Phase 3 real-time scheduling.)*

.. [fftw2005]
   Frigo, M., & Johnson, S. G. (2005). The Design and Implementation of
   FFTW3. *Proceedings of the IEEE*, 93(2), 216–231.
   https://doi.org/10.1109/JPROC.2003.823119
   *(Primary reference for the Phase 2 range-Doppler FFT pipeline.)*


.. rubric:: F — Terahertz Technology — Published Research

.. [mit_subthz]
   MIT Microsystems Technology Lab (2019). Sub-terahertz imaging method.
   *IEEE Journal of Solid State Circuits*.
   *(Chip achieving 32 pixels on 1.2 mm², 4300x sensitivity over standard
   pixels. Cited in the Teradar ADAS white paper for resolution benchmarking.)*

.. [mustang_acti]
   Mustang Technology / US Army Research Labs (2017). Terahertz Radar DVE
   Imaging — ACTI (Active Covert Terahertz Imager), 300–330 GHz.
   *(Demonstrated robust performance in dust storms, fog, and blinding
   precipitation, outperforming millimeter wave and lidar. Cited in the
   Teradar ADAS and defense white papers. Atmospheric conditions align with
   the attenuation model in* :cite:`federici2005` *.)*

.. [autosens2025]
   AutoSens Europe (2025). Terahertz Imaging — A New Category of Automotive
   Sensing Advancing ADAS and Autonomous Driving.

.. [spie2021]
   SPIE Photonics Focus (2021). Autonomous Cars Drive Terahertz Research.

.. [nrc2022]
   National Research Council Canada (2022). Sensors for Active Safety and
   Driving Automation Systems.
