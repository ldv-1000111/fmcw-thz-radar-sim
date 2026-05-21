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
