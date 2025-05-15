#ifndef DS_TREEWIDTH_SOLVER_H
#define DS_TREEWIDTH_SOLVER_H
#include <chrono>
#include <memory>

#include "../../instance.h"
#include "../solver.h"
#include "td/decomposer.h"
#include "td/nice_tree_decomposition.h"
#include "ternary.h"

namespace DSHunter {

struct ExtendedInstance : Instance {
    TreeDecomposition td;

    ExtendedInstance(const Instance &instance, TreeDecomposition td);

    void removeNode(int v) override;
};

struct TreewidthSolver {
    explicit TreewidthSolver(SolverConfig *cfg);
    // Returns true if instance was solved,
    // false if the width of found decompositions was too big to handle.
    std::optional<std::vector<int>> solve(const Instance &g);

    SolverConfig *cfg;
    std::unique_ptr<Decomposer> decomposer;

   private:
    struct BranchingEstimate {
        int depth_needed;
        int leaves;
    };

    std::vector<std::vector<int>> c;
    Instance g;
    [[nodiscard]] inline int cost(int v) const;

    NiceTreeDecomposition td;
    std::optional<std::vector<int>> solveDecomp(const Instance &instance, const TreeDecomposition &td);

    [[nodiscard]] std::pair<int, int> getWidthAndSplitter(const ExtendedInstance &instance) const;

    BranchingEstimate estimateBranching(const ExtendedInstance &instance, int depth = 0);

    int solved_leaves;
    int total_leaves;
    bool solveBranching(ExtendedInstance &instance);

    // [Parameterized Algorithms [7.3.2] - 10.1007/978-3-319-21275-3] extended to handle forced
    // edges.
    int getC(int t, TernaryFun f);

    void recoverDS(int t, TernaryFun f);
};
}  // namespace DSHunter
#endif