#ifndef BOUNDS_H
#define BOUNDS_H

#include <math.h>

#include <queue>

#include "instance.h"
namespace {

// Returns an approximation that is a upper bound on the number of additional vertices needed to
// make this instance fully dominated, calculated by a greedy algorithm always taking the maximum
// degree undominated node first.
// Complexity: O ((n + m) * log(n + m))
int greedy_upper_bound(const DSHunter::Instance &g) {
    int upper_bound = 0;
    auto nodes = g.nodes;
    std::priority_queue<std::pair<int, int>> pq;
    std::vector<int> undominated_neighbors(g.all_nodes.size(), 0);
    for (auto u : nodes) {
        if (!g.isDominated(u))
            undominated_neighbors[u]++;
        for (auto v : g.neighbourhoodExcluding(u))
            if (!g.isDominated(v))
                undominated_neighbors[u]++;

        if (!g.isTaken(u) && undominated_neighbors[u] > 0)
            pq.push({ undominated_neighbors[u], u });
    }

    std::vector<bool> dominated(g.all_nodes.size(), false);
    std::vector<bool> taken(g.all_nodes.size(), false);
    for (size_t i = 0; i < g.all_nodes.size(); i++) {
        dominated[i] = g[i].domination_status == DSHunter::DominationStatus::DOMINATED;
        taken[i] = g[i].membership_status == DSHunter::MembershipStatus::TAKEN;
    }

    auto dominate = [&](int u) {
        if (!dominated[u]) {
            dominated[u] = true;
            --undominated_neighbors[u];
            if (undominated_neighbors[u] > 0)
                pq.push({ undominated_neighbors[u], u });
            for (auto v : g.neighbourhoodExcluding(u)) {
                if (taken[v])
                    continue;
                --undominated_neighbors[v];
                if (undominated_neighbors[v] > 0)
                    pq.push({ undominated_neighbors[v], v });
            }
        }
    };

    while (!pq.empty()) {
        auto [d, v] = pq.top();
        pq.pop();
        // This might not be the minimum degree node anymore.
        if (d > undominated_neighbors[v] || taken[v])
            continue;

        dominate(v);
        taken[v] = true;
        for (auto u : g.neighbourhoodExcluding(v)) dominate(u);
        ++upper_bound;
    }

    return upper_bound;
}
}  // namespace

namespace DSHunter {

// Returns an approximation that is a lower bound on the number of additional vertices needed to
// make this instance fully dominated.
int lower_bound(const Instance &g) {
    // https://en.wikipedia.org/wiki/Set_cover_problem#Greedy_algorithm
    int ub = greedy_upper_bound(g);
    int d = 0;
    for (auto u : g.nodes) d = std::max(g.deg(u) + 1, d);
    double approx_factor = log(d) + 2;

    return ub / approx_factor;
}

// Returns an approximation that is a upper bound on the number of additional vertices needed to
// make this instance fully dominated.
int upper_bound(const Instance &g) { return greedy_upper_bound(g); }
}  // namespace DSHunter

#endif  // BOUNDS_H