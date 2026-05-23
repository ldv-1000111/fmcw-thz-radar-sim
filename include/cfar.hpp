#pragma once
#include <vector>

// Returns true if row[cell] exceeds the CA-CFAR threshold.
// guard   -- guard cells on each side of the CUT
// train   -- training cells on each side
// alpha   -- threshold multiplier (controls Pfa)
bool cfar_detect(
    const std::vector<float>& row,
    int cell, int guard, int train, float alpha
);
