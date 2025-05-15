#ifndef BOUNDS_H
#define BOUNDS_H
#include "instance.h"

namespace DSHunter {

// Returns an approximation that is a lower bound on the number of additional vertices needed to
// make this instance fully dominated.
int lowerBound(const Instance &g);

// Returns an approximation that is an upper bound on the number of additional vertices needed to
// make this instance fully dominated.
int upperBound(const Instance &g);

}  // namespace DSHunter

#endif  // BOUNDS_H