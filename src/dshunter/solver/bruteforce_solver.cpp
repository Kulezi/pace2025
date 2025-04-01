
#include "bruteforce_solver.h"

#include "../rrules.h"
#include "../utils.h"
namespace DSHunter {
void BruteforceSolver::solve(Instance &g) {
    reduce(g, reduction_rules);
    int n = g.nodeCount();
    std::vector<int> best_ds;

    for (int mask = 0; mask < (1 << n); mask++) {
        std::vector<DSHunter::NodeStatus> status = g.status;
        std::vector<int> ds;

        for (int i = 0; i < n; i++) {
            int v = g.nodes[i];
            DS_ASSERT(status[v] != TAKEN);
            if (mask >> i & 1) {
                ds.push_back(v);
                status[v] = TAKEN;
                for (auto [u, _] : g.adj[v])
                    if (status[u] == UNDOMINATED) status[u] = DOMINATED;
            }
        }

        bool is_domset = true;
        for (auto v : g.nodes) {
            if (!status[v]) {
                is_domset = false;
                break;
            }

            for (auto [w, s] : g.adj[v]) {
                if (s == FORCED && (status[v] != TAKEN && status[w] != TAKEN)) {
                    is_domset = false;
                    break;
                }
            }
        }

        if (is_domset && (best_ds.empty() || ds.size() < best_ds.size())) {
            best_ds = ds;
        }
    }

    for (auto i : best_ds) g.ds.push_back(i);
}
}  // namespace DSHunter