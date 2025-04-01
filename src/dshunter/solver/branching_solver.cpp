#include "branching_solver.h"
#include <vector>

#include "../instance.h"
#include "../rrules.h"
namespace DSHunter {

void BranchingSolver::take(Instance g, int v, std::vector<int> &best_ds, int level) {
    g.take(v);

    auto split = g.split();
    if (split.size() <= 1) {
        reduce(g, reduction_rules);
        solve(g, best_ds, level);
        return;
    }

    // TODO: each components needs at least 1 node, hence if |ds| + |#ccs| > |best| return.
    for (auto &cc : split) {
        std::vector<int> ds;
        g.nodes = cc;
        reduce(g, reduction_rules);
        solve(g, ds, level + 1);

        g.ds.insert(g.ds.end(),
                    ds.begin() + static_cast<std::vector<int>::difference_type>(g.ds.size()),
                    ds.end());
        if (!best_ds.empty() && g.ds.size() >= best_ds.size()) return;
    }

    best_ds = g.ds;
}

void BranchingSolver::solve(const Instance g, std::vector<int> &best_ds, int level) {
    throw std::logic_error("reimplement for forced edges!");
    int v = g.minDegNodeOfStatus(UNDOMINATED);
    if (!best_ds.empty() && g.ds.size() >= best_ds.size()) return;
    if (v == -1) {
        best_ds = g.ds;
        return;
    }

    // Branch 0: v belongs to DS -> dominate N(v)
    if (g.deg(v) != 1) take(g, v, best_ds, level + 1);

    // Branches 1, ..., deg(v): v doesn't belong to DS -> take any neighbour to DS.
    for (auto [u, _] : g.adj[v]) {
        take(g, u, best_ds, level + 1);
    }
}

}  // namespace DSHunter