.. _phase3_overview:

Phase 3 Overview
=================

.. list-table::
   :widths: 30 70
   :stub-columns: 1

   * - **Prerequisite**
     - Phase 2 tag ``v0.2.0`` and green CI on ``main``
   * - **Goal**
     - Package the simulator as a Yocto BitBake recipe inside a custom
       ``meta-adas-radar`` layer and cross-compile for automotive targets.
   * - **Git tag**
     - ``v0.3.0``
   * - **Target boards**
     - NXP S32G, Renesas R-Car H3, Raspberry Pi 5, QEMU x86-64

.. toctree::
   :maxdepth: 2

   meta_layer
   bitbake_recipe
   build_workflow
   profiling
   github_workflow
