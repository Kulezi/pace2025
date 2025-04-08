#ifndef DS_BRUTEFORCE_SOLVER_H
#define DS_BRUTEFORCE_SOLVER_H
#include <vector>

#include "../../instance.h"
#include "../../rrules.h"
namespace DSHunter {
struct BruteforceSolver {
    std::vector<ReductionRule> reduction_rules;

    // The minimum found dominating set is stored inside the instance.
    void solve(Instance &g);
};
}  // namespace DSHunter
#endif  // DS_BRUTEFORCE_SOLVER_H