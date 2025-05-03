#include "../rrules.h"
namespace {
using DSHunter::intersect, DSHunter::contains, DSHunter::unite, DSHunter::remove;

bool hasUndominatedNode(DSHunter::Instance& g, std::vector<int> nodes) {
    for (auto v : nodes)
        if (!g.isDominated(v))
            return true;
    return false;
}

// Checks whether node u is an exit vertex with respect to node v.
// Complexity: O(min(deg(u), deg(v)))
bool isExit(const DSHunter::Instance& g, int u, int v) {
    for (auto [w, status] : g[u].adj) {
        // This will execute at most O(deg(v)) times, since g.hasEdge(v, w) can be true only
        // for deg(v) different vertices w.
        if (w != v && (!g.hasEdge(v, w) || status == DSHunter::EdgeStatus::FORCED))
            return true;
    }

    return false;
}

// Returns a sorted list of neighbours of u that have a neighour outside of the neighbourhood of u.
// Complexity: O(deg(u)^2)
std::vector<int> exitNeighbourhood(DSHunter::Instance& g, int u) {
    std::vector<int> N_exit;
    for (auto v : g[u].n_open) {
        if (isExit(g, v, u))
            N_exit.push_back(v);
    }

    return N_exit;
}
}  // namespace
namespace DSHunter {
bool alberMainRule1(Instance& g) {
    auto nodes = g.nodes;
    bool reduced = false;

    for (auto u : nodes) {
        if (!g.hasNode(u) || g.isDisregarded(u))
            continue;

        std::vector<int> N_exit = exitNeighbourhood(g, u), N_guard, N_prison;

        for (auto v : remove(g[u].n_open, N_exit)) {
            if (!intersect(g[v].n_open, N_exit).empty())
                N_guard.push_back(v);
        }

        N_prison = remove(remove(g[u].n_open, N_exit), N_guard);

        if (!N_prison.empty() && hasUndominatedNode(g, N_prison)) {
            DS_TRACE(std::cerr << "applying " << __func__ << dbg(u) << dbgv(N_prison)
                               << dbgv(N_guard) << dbgv(N_exit) << std::endl);
            g.take(u);

            for (auto v : N_prison) g.removeNode(v);
            for (auto v : N_guard) g.removeNode(v);
            reduced = true;
        }
    }

    return reduced;
}

ReductionRule AlberMainRule1("AlberMainRule1", alberMainRule1, 3, 1);

}  // namespace DSHunter