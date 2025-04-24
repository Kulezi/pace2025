#ifndef DS_BRANCHING_SOLVER_H
#define DS_BRANCHING_SOLVER_H
#include <vector>

#include "../../instance.h"
#include "../../rrules/rrules.h"
namespace DSHunter {
struct BranchingSolver {
    std::vector<ReductionRule> reduction_rules;

    BranchingSolver();
    BranchingSolver(std::vector<ReductionRule> rrules);
    void solve(Instance g, std::vector<int> &best_ds);

   private:
    int selectNode(const Instance &g);

    void branch(const Instance &g, int v, std::vector<int> &best_ds);
};
}  // namespace DSHunter
#endif  // DS_BRANCHING_SOLVER_H