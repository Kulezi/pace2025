#ifndef BOUNDS_H
#define BOUNDS_H

#include <math.h>

#include "instance.h"
namespace bounds {

// Returns an approximation that is a upper bound on the number of additional vertices needed to
// make this instance fully dominated, calculated by a greedy algorithm always taking the maximum
// degree undominated node first. Time complexity: O (n * log m)
int greedy_upper_bound(const Instance &g) {
    int upper_bound = 0;
    auto nodes = g.nodes;
    std::priority_queue<std::pair<int, int>> pq;
    std::vector<int> deg(g.next_free_id, 0);
    for (auto u : nodes) {
        if (g.getStatus(u) == UNDOMINATED) deg[u]++;
        for (auto v : g.neighbourhoodExcluding(u))
            if (g.getStatus(v) == UNDOMINATED) deg[u]++;

        if (g.getStatus(u) != TAKEN) pq.push({deg[u], u});
    }

    auto status = g.status;
    auto dominate = [&](int u) {
        if (status[u] == UNDOMINATED) {
            status[u] = DOMINATED;
            for (auto v : g.neighbourhoodExcluding(u)) {
                if (status[v] == TAKEN) continue;
                deg[v]--;
                pq.push({deg[v], v});
            }
        }
    };

    while (!pq.empty()) {
        auto [d, v] = pq.top();
        pq.pop();
        // This might not be the minimum degree node anymore.
        if (d != deg[v] || status[v] == TAKEN) continue;

        if (status[v] == UNDOMINATED) {
            for (auto u : g.neighbourhoodExcluding(v)) dominate(u);
        }

        upper_bound++;
        status[v] = TAKEN;
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
}  // namespace

#endif  // BOUNDS_H