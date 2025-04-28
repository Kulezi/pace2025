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
    TreewidthSolver(const SolverConfig *cfg);
    // Returns true if instance was solved,
    // false if the width of found decompositions was too big to handle.
    bool solve(Instance &g);

    const SolverConfig *cfg;
    std::unique_ptr<Decomposer> decomposer;

   private:
    std::vector<std::vector<int>> c;
    Instance g;
    NiceTreeDecomposition td;
    inline int cost(const Instance &g, int v);

    // [Parameterized Algorithms [7.3.2] - 10.1007/978-3-319-21275-3] extended to handle forced
    // edges.
    int getC(int t, TernaryFun f);

    void recoverDS(int t, TernaryFun f);

    uint64_t getMemoryUsage(const NiceTreeDecomposition &td);
};
}  // namespace DSHunter
#endif