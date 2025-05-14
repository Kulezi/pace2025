#include "bounds.h"

#include <math.h>

#include <queue>

#include "instance.h"
#include "solver/heuristic/greedy.h"

namespace DSHunter {

// Returns an approximation that is a lower bound on the number of additional vertices needed to
// make this instance fully dominated.
int lowerBound(const Instance &g) {
    return static_cast<int>(maximalScatteredSet(g, 3).size());
}

// Returns an approximation that is a upper bound on the number of additional vertices needed to
// make this instance fully dominated.
int upperBound(const Instance &g) { return static_cast<int>(greedyDominatingSet(g).size()); }
}  // namespace DSHunter
