#include "bounds.h"

#include <math.h>

#include <queue>

#include "instance.h"

namespace {

// Complexity: O ((n + m) * log(n + m))
std::vector<int> greedyDominatingSet(const DSHunter::Instance &g) {
    std::vector<int> ds = g.ds;

    auto nodes = g.nodes;
    std::priority_queue<std::tuple<int, int, int>> pq;
    std::vector<int> ud(g.all_nodes.size(), 0), ufd(g.all_nodes.size());
    for (auto u : nodes) {
        for (auto v : g[u].n_closed)
            if (!g.isDominated(v))
                ud[u]++;
        for (auto [_, s] : g[u].adj)
            if (s == DSHunter::EdgeStatus::FORCED)
                ufd[u]++;

        if (ufd[u] > 0 || ud[u] > 0)
            pq.emplace(ufd[u], ud[u], u);
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
            for (auto v : g[u].n_closed) {
                pq.emplace(ufd[v], --ud[v], v);
            }
        }
    };

    while (!pq.empty()) {
        auto [ufd_v, ud_v, v] = pq.top();
        pq.pop();
        // This might not be the minimum degree node anymore.
        if (taken[v] || ufd_v > ufd[v] || ud_v > ud[v] || (ufd_v == 0 && ud_v == 0))
            continue;

        dominate(v);
        for (auto [u, s] : g[v].adj) {
            if (s == DSHunter::EdgeStatus::FORCED) {
                pq.emplace(--ufd[u], ud[u], u);
            }
        }

        ds.push_back(v);
        taken[v] = true;
        for (auto u : g[v].n_closed) dominate(u);
    }


    return ds;
}

// Returns a maximal (not the largest one!) set of nodes that are at distances at least d from each other.
// Complexity: O ((n + m) * log(n + m))
std::vector<int> maximal_scattered_set(const DSHunter::Instance &g, int d) {
    std::vector<int> res;
    std::vector<int> dis(g.all_nodes.size(), g.all_nodes.size());
    for (auto u : g.nodes) {
        if (dis[u] <= d)
            continue;
        res.push_back(u);
        dis[u] = 0;

        std::queue<int> q;
        q.push(u);
        while (!q.empty()) {
            int v = q.front();
            q.pop();

            if (dis[v] >= d)
                continue;
            for (auto w : g[v].n_open) {
                if (dis[w] > dis[v] + 1) {
                    dis[w] = dis[v] + 1;
                    q.push(w);
                }
            }
        }
    }

    return res;
}
}  // namespace

namespace DSHunter {

// Returns an approximation that is a lower bound on the number of additional vertices needed to
// make this instance fully dominated.
int lowerBound(const Instance &g) {
    return static_cast<int>(maximal_scattered_set(g, 3).size());
}

// Returns an approximation that is a upper bound on the number of additional vertices needed to
// make this instance fully dominated.
int upperBound(const Instance &g) { return static_cast<int>(greedyDominatingSet(g).size()); }
}  // namespace DSHunter
