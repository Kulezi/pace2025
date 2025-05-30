#include "../rrules.h"
namespace {
using DSHunter::intersect, DSHunter::contains, DSHunter::unite, DSHunter::remove, DSHunter::Instance;
using std::vector;

bool hasUndominatedNode(Instance& g, const vector<int>& nodes) {
    return std::ranges::any_of(nodes, std::not_fn(g.isDominated));
}

// Checks whether node u is an exit vertex with respect to node v.
// Complexity: O(min(deg(u), deg(v)))
bool isExit(const Instance& g, const int u, const int v) {
    return std::ranges::any_of(g[u].adj, [&](const DSHunter::Endpoint e) {
        return e.to != v && (!g.hasEdge(v, e.to) || e.status == DSHunter::EdgeStatus::FORCED);
    });
}

// Returns a sorted list of u's neighbors that have a neighbor outside the neighborhood of u.
// Complexity: O(deg(u)^2)
vector<int> exitNeighbourhood(Instance& g, int u) {
    vector<int> N_exit;
    for (auto v : g[u].n_open) {
        if (isExit(g, v, u))
            N_exit.push_back(v);
    }

    return N_exit;
}
}  // namespace
namespace DSHunter {
bool alberMainRule1(Instance& g) {
    const auto nodes = g.nodes;
    bool reduced = false;

    for (const auto u : nodes) {
        if (!g.hasNode(u) || g.isDisregarded(u))
            continue;

        vector<int> N_exit = exitNeighbourhood(g, u), N_guard, N_prison;

        for (auto v : remove(g[u].n_open, N_exit)) {
            if (!intersect(g[v].n_open, N_exit).empty())
                N_guard.push_back(v);
        }

        N_prison = remove(remove(g[u].n_open, N_exit), N_guard);

        if (!N_prison.empty() && hasUndominatedNode(g, N_prison)) {
            DS_TRACE(std::cerr << "applying " << __func__ << dbg(u) << dbgv(N_prison)
                               << dbgv(N_guard) << dbgv(N_exit) << std::endl);
            g.take(u);

            g.removeNodes(N_prison);
            g.removeNodes(N_guard);
            reduced = true;
        }
    }

    return reduced;
}

ReductionRule AlberMainRule1("AlberMainRule1", alberMainRule1, 3, 1);

}  // namespace DSHunter