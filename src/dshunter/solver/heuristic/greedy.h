#ifndef DS_GREEDY_H
#define DS_GREEDY_H

#include "dshunter/instance.h"

namespace DSHunter {
std::vector<int> greedyDominatingSet(const Instance &g);
std::vector<int> maximalScatteredSet(const Instance &g, int d);
}  // namespace DSHunter

#endif  // DS_GREEDY_H
