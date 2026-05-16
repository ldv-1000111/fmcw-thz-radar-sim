.. _formulas:

Formula Quick-Reference
========================

FMCW Key Equations
-------------------

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Metric
     - Formula
   * - Range resolution
     - :math:`\Delta r = c / (2B)`
   * - Max unambiguous range
     - :math:`R_\text{max} = f_s \cdot c \;/\; (2 B/T_c)`
   * - Doppler (velocity) resolution
     - :math:`\Delta v = \lambda \;/\; (2 N_c T_c)`
   * - Max unambiguous velocity
     - :math:`v_\text{max} = \lambda \;/\; (4 T_c)`
   * - Beat frequency
     - :math:`f_\text{beat} = (B/T_c)\,\tau + 2 f_0 v / c`
   * - Round-trip delay
     - :math:`\tau = 2R/c`
   * - Range bin index
     - :math:`k_r = \operatorname{round}(f_\text{beat} \cdot N_s / f_s)`
   * - Doppler bin index
     - :math:`k_d = \operatorname{round}(f_\text{Doppler} \cdot N_c \cdot T_c)`

Example: 300 GHz THz System
-----------------------------

.. list-table::
   :header-rows: 1
   :widths: 25 20 55

   * - Parameter
     - Value
     - Derived
   * - :math:`f_0`
     - 300 GHz
     - :math:`\lambda = 1\,\text{mm}`
   * - :math:`B`
     - 4 GHz
     - :math:`\Delta r = 3.75\,\text{cm}`
   * - :math:`T_c`
     - 100 µs
     - —
   * - :math:`N_c`
     - 256 chirps
     - :math:`\Delta v = 1.95\,\text{mm/s}`
   * - :math:`f_s`
     - 50 MHz
     - :math:`N_s = 5000\,\text{samples/chirp}`
   * - Frame rate
     - ≈ 39 Hz
     - :math:`1 / (N_c \cdot T_c)`
