
#include "bruteforce_solver.h"

#include "../../utils.h"

namespace DSHunter {
std::vector<int> BruteforceSolver::solve(Instance g) {
    std::vector<int> nodes;
    for (auto v : g.nodes) if (!g.isDisregarded(v)) nodes.push_back(v);

    int n = static_cast<int>(nodes.size());
    std::vector<int> best_ds;

    for (int mask = 0; mask < (1 << n); mask++) {
        std::vector dominated(g.all_nodes.size(), false);
        std::vector taken(g.all_nodes.size(), false);

        for (size_t i = 0; i < g.all_nodes.size(); i++) {
            dominated[i] = g[i].domination_status == DominationStatus::DOMINATED;
            taken[i] = g[i].membership_status == MembershipStatus::TAKEN;
        }

        std::vector<int> ds;

        for (int i = 0; i < n; i++) {
            int v = nodes[i];
            DS_ASSERT(!taken[v]);
            if (mask >> i & 1) {
                ds.push_back(v);
                taken[v] = dominated[v] = true;
                for (auto u : g[v].n_open)
                    dominated[u] = true;
            }
        }

        bool is_domset = true;
        for (auto v : g.nodes) {
            if (!dominated[v]) {
                is_domset = false;
                break;
            }

            for (auto [w, s] : g[v].adj) {
                if (s == EdgeStatus::FORCED && (!taken[v] && !taken[w])) {
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
    return g.ds;
}
}  // namespace DSHunter