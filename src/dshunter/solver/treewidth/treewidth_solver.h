#ifndef DS_TREEWIDTH_SOLVER_H
#define DS_TREEWIDTH_SOLVER_H
#include <chrono>
#include <cstdint>
#include <memory>

#include "../../instance.h"
#include "../solver.h"
#include "td/decomposer.h"
#include "td/nice_tree_decomposition.h"
#include "ternary.h"

namespace DSHunter {
struct TreewidthSolver {
    TreewidthSolver(SolverConfig *cfg);
    // Returns true if instance was solved,
    // false if the width of found decompositions was too big to handle.
    bool solve(Instance &g);

    SolverConfig *cfg;
    std::unique_ptr<Decomposer> decomposer;

   private:
    struct BranchingEstimate {
        int depth_needed;
        int leaves;
    };

    std::vector<std::vector<int>> c;
    Instance g;
    inline int cost(int v);

    NiceTreeDecomposition td;
    bool solveDecomp(Instance &instance, TreeDecomposition td);

    BranchingEstimate estimateBranching(Instance &instance, TreeDecomposition td, int depth = 0);

    int solved_leaves;
    int total_leaves;
    bool solveBranching(Instance &instance, TreeDecomposition td);

    // [Parameterized Algorithms [7.3.2] - 10.1007/978-3-319-21275-3] extended to handle forced
    // edges.
    int getC(int t, TernaryFun f);

    void recoverDS(int t, TernaryFun f);
};
}  // namespace DSHunter
#endif