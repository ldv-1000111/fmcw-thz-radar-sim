.. _frameworks:

Open-Source Frameworks
=======================

FERS — Flexible Extensible Radar Simulator
-------------------------------------------

Developed by Marc Brooker and Prof. Michael Inggs at the University of Cape
Town Radar Remote Sensing Group. FERS is the most mature open-source
signal-level radar simulator written in C++. It supports monostatic, bistatic,
and multistatic configurations in both pulsed and CW modes.

* **GitHub:** https://github.com/stpaine/FERS
* **License:** GNU GPL v2
* **Key use:** Reference architecture for XML-to-C++ scene representation and
  IF signal generation loop structure.

C-Shenron — Physics-Based Radar for CARLA
-------------------------------------------

C-Shenron (UC San Diego / BITS Pilani, IEEE VTC 2025) integrates a realistic
radar sensor into the CARLA autonomous driving simulator, generating full ADC
data cubes from LiDAR point clouds and camera semantic segmentation.

* **GitHub:** https://github.com/ucsdwcsng/c-shenron
* **DOI:** `10.1109/VTC2025-Fall65116.2025.11310463 <https://doi.org/10.1109/VTC2025-Fall65116.2025.11310463>`_

FFTW3
------

The de-facto open-source C library for high-performance FFTs, generating
architecture-specific SIMD code (ARM NEON, x86 AVX2) at plan time.

* **Website:** https://www.fftw.org/
* **Yocto dependency:** ``DEPENDS = "fftw"``
* **Key functions:** ``fftwf_plan_dft_1d()``, ``fftwf_execute()``,
  ``fftwf_destroy_plan()``

NVIDIA OptiX Ray-Tracing Simulator (RTS)
-----------------------------------------

Open-source RTS module by Martin et al. (2022) using NVIDIA OptiX for
shoot-and-bounce ray tracing of transmitted radar signals.

* **GitHub:** https://github.com/ymartin101/RTS
* **Note:** Must be integrated into an existing simulator; cannot run standalone.

Yocto Project
--------------

The Linux Foundation's open-source framework for building custom embedded Linux
distributions.

* **Website:** https://www.yoctoproject.org/
* **Docs:** https://docs.yoctoproject.org/
* **Current LTS branch:** ``scarthgap`` (Yocto 5.x, 2024–2026)
