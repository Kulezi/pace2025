#ifndef BOUNDS_H
#define BOUNDS_H

#include <math.h>

#include "instance.h"
namespace bounds {

// Returns an approximation that is a upper bound on the number of additional vertices needed to
// make this instance fully dominated, calculated by a greedy algorithm always taking the maximum
// degree undominated node first. 
// Complexity: O ((n + m) * log(n + m))
int greedy_upper_bound(const Instance &g) {
    int upper_bound = 0;
    auto nodes = g.nodes;
    std::priority_queue<std::pair<int, int>> pq;
    std::vector<int> undominated_neighbors(g.next_free_id, 0);
    for (auto u : nodes) {
        if (g.getStatus(u) == UNDOMINATED) undominated_neighbors[u]++;
        for (auto v : g.neighbourhoodExcluding(u))
            if (g.getStatus(v) == UNDOMINATED) undominated_neighbors[u]++;

        if (g.getStatus(u) != TAKEN && undominated_neighbors[u] > 0)
            pq.push({undominated_neighbors[u], u});
    }

    auto status = g.status;
    auto dominate = [&](int u) {
        if (status[u] == UNDOMINATED) {
            status[u] = DOMINATED;
            --undominated_neighbors[u];
            if (undominated_neighbors[u] > 0) pq.push({undominated_neighbors[u], u});
            for (auto v : g.neighbourhoodExcluding(u)) {
                if (status[v] == TAKEN) continue;
                --undominated_neighbors[v];
                if (undominated_neighbors[v] > 0) pq.push({undominated_neighbors[v], v});
            }
        }
    };

    while (!pq.empty()) {
        auto [d, v] = pq.top();
        pq.pop();
        // This might not be the minimum degree node anymore.
        if (d > undominated_neighbors[v] || status[v] == TAKEN) continue;

        dominate(v);
        status[v] = TAKEN;
        for (auto u : g.neighbourhoodExcluding(v)) dominate(u);
        ++upper_bound;
    }

    return upper_bound;
}

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
}  // namespace bounds

#endif  // BOUNDS_H