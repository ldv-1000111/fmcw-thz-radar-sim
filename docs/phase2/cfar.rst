.. _phase2_cfar:

CA-CFAR Detection
==================

Cell-Averaging Constant False Alarm Rate (CA-CFAR) sets an adaptive detection
threshold based on the local noise level in the Range-Doppler map.

.. code-block:: cpp
   :caption: include/cfar.hpp
   :linenos:

   #pragma once
   #include <vector>

   // Returns true if rd_map[row][cell] exceeds the CA-CFAR threshold.
   // guard   — guard cells on each side of the CUT
   // train   — training cells on each side
   // alpha   — threshold multiplier (controls Pfa)
   bool cfar_detect(
       const std::vector<float>& row,
       int cell, int guard, int train, float alpha
   );

.. code-block:: cpp
   :caption: src/cfar.cpp
   :linenos:

   #include "cfar.hpp"
   #include <algorithm>
   #include <cmath>

   bool cfar_detect(
       const std::vector<float>& row,
       int cell, int guard, int train, float alpha)
   {
       int   N     = static_cast<int>(row.size());
       int   lo    = std::max(0, cell - guard - train);
       int   hi    = std::min(N - 1, cell + guard + train);
       float noise = 0.0f;
       int   count = 0;

       for (int k = lo; k <= hi; ++k) {
           if (std::abs(k - cell) > guard)
               noise += row[k], ++count;
       }

       float threshold = (count > 0) ? alpha * (noise / count) : 0.0f;
       return row[cell] > threshold;
   }
