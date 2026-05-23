.. _phase2_cfar:

CA-CFAR Detection
==================

Cell-Averaging Constant False Alarm Rate (CA-CFAR) sets an adaptive
threshold around each Cell Under Test (CUT) based on the mean energy of
surrounding training cells, excluding a guard band. This keeps the false
alarm rate constant regardless of local noise floor variations.

.. todo::

   This page will be completed once ``v0.2.0`` is tagged on ``main``.

----

Algorithm
----------

For a CUT at index ``cell`` in a row of the Range-Doppler map:

1. Exclude ``guard`` cells on each side of the CUT from the noise estimate
2. Average the ``train`` cells on each side beyond the guard band
3. Multiply the average by ``alpha`` to get the threshold
4. Return ``true`` if ``row[cell] > threshold``

.. code-block:: text

   |--- train ---|--- guard ---|  CUT  |--- guard ---|--- train ---|
                                  ^
                             cell under test

``include/cfar.hpp``
---------------------

.. code-block:: cpp
   :caption: include/cfar.hpp
   :linenos:

   #pragma once
   #include <vector>

   // Returns true if rd_map[row][cell] exceeds the CA-CFAR threshold.
   // guard   -- guard cells on each side of the CUT
   // train   -- training cells on each side
   // alpha   -- threshold multiplier (controls Pfa)
   bool cfar_detect(
       const std::vector<float>& row,
       int cell, int guard, int train, float alpha
   );

----

``src/cfar.cpp``
-----------------

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
       const int   N     = static_cast<int>(row.size());
       const int   lo    = std::max(0,     cell - guard - train);
       const int   hi    = std::min(N - 1, cell + guard + train);
       float       noise = 0.0f;
       int         count = 0;

       for (int k = lo; k <= hi; ++k) {
           if (std::abs(k - cell) > guard)
               noise += row[k], ++count;
       }

       const float threshold = (count > 0) ? alpha * (noise / count) : 0.0f;
       return row[cell] > threshold;
   }

----

Choosing Parameters
--------------------

.. list-table::
   :header-rows: 1
   :widths: 20 20 60

   * - Parameter
     - Typical value
     - Effect
   * - ``guard``
     - 2–4 bins
     - Prevents the target's own energy leaking into the noise estimate.
       Too small → threshold inflated by target sidelobes.
   * - ``train``
     - 8–16 bins
     - More training cells → more stable noise estimate.
       Too few → noisy threshold; too many → averages in distant clutter.
   * - ``alpha``
     - 1.5–4.0
     - Higher alpha → fewer false alarms but lower detection probability.
       Tune against the expected SNR of the weakest target.
