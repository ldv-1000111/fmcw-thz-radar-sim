.. FMCW & TeraHertz Radar Simulation documentation master file
   Author: Luis Viveros — May 2026

FMCW & TeraHertz Radar Simulation
===================================

.. image:: https://img.shields.io/badge/build-passing-brightgreen
   :alt: Build Status

.. image:: https://img.shields.io/badge/tests-passing-brightgreen
   :alt: Tests

.. image:: https://img.shields.io/badge/C%2B%2B-17-blue
   :alt: C++17

.. image:: https://img.shields.io/badge/version-0.1.0-orange
   :alt: Version 0.1.0

.. image:: https://img.shields.io/badge/license-MIT-lightgrey
   :alt: MIT License

|

**Author:** Luis Viveros · **Date:** May 2026 · **Repository:** `fmcw-terahertz-radar-simulation <https://github.com/ldv-1000111/fmcw-terahertz-radar-simulation>`_

----

A complete engineering tutorial covering the simulation of FMCW and Terahertz
radar systems in C++17 and their deployment to embedded Linux targets via the
Yocto Project build system. The tutorial is structured in three phases, each
with a self-contained implementation, Catch2 test suite, and GitHub Actions CI
pipeline.

.. rubric:: What you will build

* A **physics-level FMCW IF signal generator** with THz micro-Doppler support
* A **2D Range-Doppler processing pipeline** using FFTW3 and CA-CFAR detection
* A **Yocto meta-layer** that cross-compiles and deploys the simulator to NXP S32G,
  Renesas R-Car, and Raspberry Pi 5 targets

.. rubric:: Prerequisites

* C++17 compiler (GCC ≥ 11 or Clang ≥ 14)
* CMake ≥ 3.24
* Python 3.10+ with ``numpy`` and ``matplotlib`` (for validation scripts)
* Git ≥ 2.38
* Basic familiarity with radar fundamentals (bandwidth, chirp, beat frequency)

.. toctree::
   :maxdepth: 2
   :caption: Getting Started

   introduction
   sensor_landscape

.. toctree::
   :maxdepth: 2
   :caption: Phase 1 — Physics Engine

   phase1/overview
   phase1/repo_setup
   phase1/project_structure
   phase1/fmcw_theory
   phase1/cpp_implementation
   phase1/cmake_build
   phase1/testing
   phase1/python_validation
   phase1/github_workflow

.. toctree::
   :maxdepth: 2
   :caption: Phase 2 — Signal Processing

   phase2/overview
   phase2/data_cube
   phase2/range_doppler
   phase2/cfar
   phase2/embedded_opt
   phase2/github_workflow

.. toctree::
   :maxdepth: 2
   :caption: Phase 3 — Yocto Deployment

   phase3/overview
   phase3/meta_layer
   phase3/bitbake_recipe
   phase3/build_workflow
   phase3/profiling
   phase3/github_workflow

.. toctree::
   :maxdepth: 2
   :caption: Milestones

   milestones

.. toctree::
   :maxdepth: 2
   :caption: Reference

   reference/formulas
   reference/frameworks
   reference/bibliography

.. toctree::
   :maxdepth: 1
   :caption: About

   changelog
   license
