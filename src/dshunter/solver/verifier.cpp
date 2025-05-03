#ifndef DS_VERIFIER_H
#define DS_VERIFIER_H
#include "../instance.h"
#include "../utils.h"
namespace DSHunter {

// Checks whether the given solution is a valid dominating set of the given instance.
// Throws a std::logic_error if it isn't.
void verify_solution(Instance g, const std::vector<int> solution) {
    std::vector<bool> dominated(g.all_nodes.size(), false);
    std::vector<bool> taken(g.all_nodes.size(), false);
    for (auto u : solution) {
        if (taken[u])
            throw std::logic_error("solution contains duplicates, one of which is vertex " +
                                   std::to_string(u));

        taken[u] = true;
        for (auto v : g[u].n_closed) dominated[v] = true;
    }

    for (auto u : g.nodes) {
        if (!dominated[u])
            throw std::logic_error("solution doesn't dominate vertex " + std::to_string(u));
        for (auto [v, s] : g[u].adj) {
            if (s == EdgeStatus::FORCED && !taken[u] && !taken[v])
                throw std::logic_error("forced edge (" + std::to_string(u) + ", " +
                                       std::to_string(v) + ") is unsatisfied");
        }
    }
}
}  // namespace DSHunter
#endif  // DS_VERIFIER_H