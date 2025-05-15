#ifndef DS_BRANCHING_SOLVER_H
#define DS_BRANCHING_SOLVER_H
#include <vector>

#include "../../instance.h"
#include "../../rrules/rrules.h"
#include "../solver.h"
namespace DSHunter {
struct BranchingSolver {
    std::vector<ReductionRule> reduction_rules;
    SolverConfig *cfg;

    explicit BranchingSolver(SolverConfig *cfg);
    std::vector<int> solve(const Instance &g);

   private:
    static int selectNode(const Instance &g);
    void solve(Instance g, std::vector<int> &best_ds);
    void branch(Instance &g, std::vector<int> &best_ds);
};
}  // namespace DSHunter
#endif  // DS_BRANCHING_SOLVER_H