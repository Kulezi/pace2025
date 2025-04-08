#ifndef DS_TREEWIDTH_SOLVER_H
#define DS_TREEWIDTH_SOLVER_H
#include <cstdint>
#include "../../instance.h"
#include "td/nice_tree_decomposition.h"
#include "ternary.h"

namespace DSHunter {
constexpr uint64_t MAX_MEMORY_IN_BYTES = (1UL << 30);
constexpr size_t MAX_HANDLED_TREEWIDTH = 18;
constexpr size_t GOOD_ENOUGH_TREEWIDTH = 15;
struct TreewidthSolver {
    std::vector<std::vector<int>> c;

    // Returns true if instance was solved,
    // false if the width of found decompositions was too big to handle.
    bool solve(Instance &g);

   private:
    inline int cost(const Instance &g, int v);

    // [Parameterized Algorithms [7.3.2] - 10.1007/978-3-319-21275-3] extended to handle forced
    // edges.
    int getC(const Instance &g, NiceTreeDecomposition &td, int t, TernaryFun f);

    void recoverDS(Instance &g, NiceTreeDecomposition &td, int t, TernaryFun f);

    uint64_t getMemoryUsage(const NiceTreeDecomposition &td);
};
}  // namespace DSHunter
#endif