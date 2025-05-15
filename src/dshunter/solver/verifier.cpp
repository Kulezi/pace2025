#ifndef DS_VERIFIER_H
#define DS_VERIFIER_H
#include <format>

#include "../instance.h"
#include "../utils.h"
namespace DSHunter {

void verify_solution(const Instance &g, const std::vector<int> &solution) {
    std::vector dominated(g.all_nodes.size(), false);
    for (auto u : g.nodes) {
        if (g.isDominated(u)) dominated[u] = true;
    }

    std::vector taken(g.all_nodes.size(), false);
    for (auto u : solution) {
        if (g.isDisregarded(u)) throw std::logic_error(std::format("solution contains disregarded vertex {}", u));
        if (taken[u])
            throw std::logic_error(std::format("solution contains duplicates, one of which is vertex {}", u));
        taken[u] = true;
        for (auto v : g[u].n_closed) dominated[v] = true;
    }

    for (auto u : g.nodes) {
        if (!dominated[u])
            throw std::logic_error(std::format("solution doesn't dominate vertex {}", u));
        for (auto [v, s] : g[u].adj) {
            if (s == EdgeStatus::FORCED && !taken[u] && !taken[v])
                throw std::logic_error(std::format("forced edge ({}, {}) is unsatisfied", u, v));
        }
    }
}
}  // namespace DSHunter
#endif  // DS_VERIFIER_H