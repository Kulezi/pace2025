#ifndef DS_BRANCHING_SOLVER_H
#define DS_BRANCHING_SOLVER_H
#include <vector>

#include "../instance.h"
#include "../rrules.h"
namespace DSHunter {
struct BranchingSolver {
    std::vector<ReductionRule> reduction_rules;

    void solve(const Instance g, std::vector<int> &best_ds, int level = 0);

   private:
    void take(Instance g, int v, std::vector<int> &best_ds, int level);
};
}  // namespace DSHunter
#endif  // DS_BRANCHING_SOLVER_H