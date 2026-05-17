.. _introduction:

Introduction & Motivation
=========================

.. list-table::
   :widths: 20 80
   :stub-columns: 1

   * - **Author**
     - Luis Viveros
   * - **Date**
     - May 2026
   * - **License**
     - MIT — see the :ref:`license` page for the full text.
       All source code and documentation in this tutorial may be freely
       used, modified, and redistributed under the terms of the
       `MIT License <https://opensource.org/licenses/MIT>`_.

----

Advanced Driver Assistance Systems (ADAS) and fully autonomous vehicles require
sensing technologies that perform reliably in every operational design domain —
in fog, rain, snow, direct sunlight glare, and dust. Cameras excel in clear
conditions; LiDAR provides dense 3D point clouds — but both degrade severely in
adverse weather. Conventional 77 GHz millimeter-wave radar offers all-weather
robustness but lacks the spatial resolution needed for precise object
classification.

Terahertz (THz) imaging, operating between **0.1–10 THz** (wavelengths
3 mm–30 µm), bridges this gap. THz vision delivers sub-millimeter native
angular resolution, multi-material fingerprinting, immunity to optical glare,
and inherent non-ionizing safety — all within a solid-state, chip-scale
architecture designed for automotive-grade mass production.

.. note::

   This tutorial is based on peer-reviewed literature and Teradar's validated
   hardware specifications. All claims are traceable to the
   :ref:`bibliography <bibliography>`.

Why Simulate Before You Fabricate?
------------------------------------

High-fidelity radar simulation allows engineers to:

* Prototype signal processing algorithms before silicon is available
* Test CFAR detectors across a full range of SNR and clutter conditions
* Generate labelled synthetic training data for neural network classifiers
* Validate embedded real-time budgets on QEMU before flashing real hardware

The Schasler et al. (2021) ray-tracing simulator demonstrated that simulation
can produce training data for MIMO arrays across a full suite of automotive
edge cases. Liu et al. (2022) showed that CPU/GPU co-processing can accelerate
radar data cubes by 13 % over GPU-only architectures.

.. rubric:: Tutorial Scope

This tutorial covers the **simulation** and **embedded deployment** pipeline.
It does not cover RF front-end hardware design, antenna placement, or regulatory
approval processes.

.. seealso::

   * :ref:`sensor_landscape` — comparative table of ADAS sensor modalities
   * :ref:`phase1_overview` — start building immediately
