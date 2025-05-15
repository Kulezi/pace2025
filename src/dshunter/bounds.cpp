#include "bounds.h"

#include <math.h>

#include <queue>

#include "instance.h"
#include "solver/heuristic/greedy.h"

namespace DSHunter {

int lowerBound(const Instance &g) {
    return static_cast<int>(maximalScatteredSet(g, 3).size());
}

int upperBound(const Instance &g) { return static_cast<int>(greedyDominatingSet(g).size()); }
}  // namespace DSHunter
