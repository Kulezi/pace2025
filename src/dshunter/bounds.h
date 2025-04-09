#ifndef BOUNDS_H
#define BOUNDS_H
#include "instance.h"

namespace DSHunter {

// Returns an approximation that is a lower bound on the number of additional vertices needed to
// make this instance fully dominated.
int lower_bound(const Instance &g);

// Returns an approximation that is a upper bound on the number of additional vertices needed to
// make this instance fully dominated.
int upper_bound(const Instance &g);

}  // namespace DSHunter

#endif  // BOUNDS_H