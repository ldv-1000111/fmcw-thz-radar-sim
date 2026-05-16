.. _sensor_landscape:

Sensor Technology Landscape
============================

The table below summarises the four primary ADAS sensing modalities.
Terahertz radar occupies a unique position: it inherits the all-weather
robustness of conventional radar while delivering resolution that rivals
or surpasses LiDAR.

.. list-table:: ADAS Sensor Comparison
   :header-rows: 1
   :widths: 18 18 22 24 18

   * - Sensor
     - Resolution
     - Weather
     - Data Output
     - Cost
   * - Camera
     - High (visual)
     - Fails in fog / night
     - RGB frames
     - Low
   * - LiDAR
     - High 3D
     - Fails in rain / fog
     - Dense point cloud
     - Very High
   * - mmWave Radar (77 GHz)
     - Low–Medium
     - All-weather
     - Range + Velocity
     - Medium
   * - **THz Radar (300+ GHz)**
     - **Sub-mm native**
     - **All-weather + glare-immune**
     - Range + Vel + Material ID
     - Medium (chip-scale)

Teradar Validated Performance
-------------------------------

From the Teradar ADAS Whitepaper (2025) and peer-reviewed sources:

* **Angular resolution:** 0.5° at 160 m+ (peer-reviewed validation)
* **Native angular resolution:** 0.1° (Modular Terahertz Engine specification)
* **Range resolution:** ±1.5 cm at up to 300 m
* **Field of view:** 120 × 30 degrees, extendable by digital beam steering
* **Collision avoidance response:** sub-400 ms in thick fog where LiDAR and
  cameras failed (MIT / Mustang Technologies Army Research Labs validation)
* **Production roadmap:** prototype availability US & Europe 2026;
  global high-volume 2028

.. note::

   MIT's sub-terahertz array chip achieves up to 32 pixels on a 1.2 mm²
   device — 4300× more sensitive than standard sensor pixels — delivering
   robust imaging in fog and dust where LiDAR is ineffective.
