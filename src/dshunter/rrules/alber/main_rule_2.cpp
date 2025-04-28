#include <limits.h>

#include <queue>

#include "../rrules.h"

namespace {
using DSHunter::intersect, DSHunter::contains, DSHunter::unite, DSHunter::remove;
constexpr int BFS_INF = INT_MAX;

bool applyCase1_1(DSHunter::Instance& g, int v, int w, const std::vector<int>& N_prison, const std::vector<int>& N_guard, const std::vector<int>& N_v_without, const std::vector<int>& N_w_without) {
    if (!g.hasEdge(v, w)) {
        // Don't apply the reduction if it doesn't reduce the size of the graph.
        if (N_prison.size() + intersect(intersect(N_guard, N_v_without), N_w_without).size() <= 3)
            return false;
        DS_TRACE(std::cerr << "applying " << __func__ << " (with gadget vertex) " << dbg(v)
                           << dbg(w) << dbgv(N_prison) << dbgv(N_guard) << dbgv(N_v_without)
                           << dbgv(N_w_without) << std::endl);
        int z1 = g.addNode();
        int z2 = g.addNode();
        int z3 = g.addNode();

        g.addEdge(v, z1);
        g.addEdge(v, z2);
        g.addEdge(v, z3);

        g.addEdge(w, z1);
        g.addEdge(w, z2);
        g.addEdge(w, z3);

        g.removeNodes(N_prison);
        g.removeNodes(intersect(intersect(N_guard, N_v_without), N_w_without));
        return true;
    }
    DS_TRACE(std::cerr << "applying " << __func__ << " (edge forcing) " << dbg(v) << dbg(w)
                       << dbgv(N_prison) << dbgv(N_guard) << dbgv(N_v_without) << dbgv(N_w_without)
                       << std::endl);
    // The edge was already there, but could be forced already.
    if (g.getEdgeStatus(v, w) == DSHunter::EdgeStatus::UNCONSTRAINED) {
        g.forceEdge(v, w);
    }
    g.removeNodes(N_prison);
    g.removeNodes(intersect(intersect(N_guard, N_v_without), N_w_without));

    return true;
}

bool applyCase1_2(DSHunter::Instance& g, int v, const std::vector<int>& N_prison, const std::vector<int>& N_v_without, const std::vector<int>& N_guard) {
    DS_TRACE(std::cerr << "applying " << __func__ << dbg(v) << dbgv(N_prison) << dbgv(N_guard)
                       << dbgv(N_v_without) << std::endl);
    g.take(v);
    g.removeNodes(N_prison);
    g.removeNodes(intersect(N_v_without, N_guard));
    return true;
}

bool applyCase1_3(DSHunter::Instance& g, int w, const std::vector<int>& N_prison, const std::vector<int>& N_w_without, const std::vector<int>& N_guard) {
    DS_TRACE(std::cerr << "applying " << __func__ << dbg(w) << dbgv(N_prison) << dbgv(N_guard)
                       << dbgv(N_w_without) << std::endl);
    g.take(w);
    g.removeNode(w);
    g.removeNodes(N_prison);
    g.removeNodes(intersect(N_w_without, N_guard));
    return true;
}

bool applyCase2(DSHunter::Instance& g, int v, int w, const std::vector<int>& N_vw_without, const std::vector<int>& N_prison, const std::vector<int>& N_guard) {
    DS_TRACE(std::cerr << "applying " << __func__ << dbg(v) << dbg(w) << dbgv(N_prison)
                       << dbgv(N_guard) << std::endl);
    // w might get removed when taking v, so we need to set statuses before taking v and w.
    for (auto u : N_vw_without) {
        g.markDominated(u);
    }
    g.take(v);
    g.take(w);

    g.removeNodes(N_prison);
    g.removeNodes(N_guard);

    return true;
}

// Checks whether node u is an exit vertex with respect
// to the closed neighbourhood of nodes v and w.
// Vertices with a red edge pointing to somewhere else than v and w
// are considered exit vertices, since the edge means there was an additional vertex
// that would be reachable outside of the neighbourhood, hence we would be able to exit.
// Complexity: O(min(deg(u), deg(v) + deg(w))
bool isExit(const DSHunter::Instance& g, int u, int v, int w) {
    for (auto [x, status] : g[u].adj) {
        // This will execute at most O(deg(v) + deg(w)) times, since g.hasEdge(x, ?) can be true
        // only for deg(v) + deg(w) different vertices x.
        if (x != v && x != w &&
            ((!g.hasEdge(x, v) && !g.hasEdge(x, w)) || status == DSHunter::EdgeStatus::FORCED))
            return true;
    }

    return false;
}

void populateExitNodes(const DSHunter::Instance& g, const std::vector<int>& N_vw_without, int v, int w, std::vector<int>& N_exit) {
    for (auto u : N_vw_without) {
        if (isExit(g, u, v, w)) {
            N_exit.push_back(u);
        }
    }
}

void populateGuardNodes(const DSHunter::Instance& g, const std::vector<int>& N_vw_without, const std::vector<int>& N_exit, std::vector<int>& N_guard) {
    for (auto u : remove(N_vw_without, N_exit)) {
        auto N_u = g.neighbourhoodExcluding(u);
        if (!intersect(N_u, N_exit).empty()) {
            N_guard.push_back(u);
        }
    }
}

std::vector<int> redNeighbours(DSHunter::Instance& g, int v) {
    std::vector<int> res;
    for (auto [u, status] : g[v].adj)
        if (status == DSHunter::EdgeStatus::FORCED)
            res.push_back(u);
    return res;
}

std::vector<int> filterDominatedNodes(const DSHunter::Instance& g,
                                      const std::vector<int>& N_prison) {
    std::vector<int> N_prison_intersect_B = N_prison;
    const auto new_end = std::remove_if(N_prison_intersect_B.begin(), N_prison_intersect_B.end(), [&](int u) { return g.isDominated(u); });
    N_prison_intersect_B.erase(new_end, N_prison_intersect_B.end());
    return N_prison_intersect_B;
}

bool canBeDominatedBySingleNode(const DSHunter::Instance& g,
                                const std::vector<int>& N_prison_intersect_B,
                                const std::vector<int>& N_guard,
                                const std::vector<int>& N_prison) {
    auto intersection_can_be_dominated_by_single_from = [&](const std::vector<int>& nodes) {
        for (auto x : nodes)
            if (contains(g.neighbourhoodIncluding(x), N_prison_intersect_B))
                return true;
        return false;
    };
    return N_prison_intersect_B.empty() || intersection_can_be_dominated_by_single_from(N_guard) ||
           intersection_can_be_dominated_by_single_from(N_prison);
}

// Tries to apply Main Rule 2 for a given pair of vertices - DOI 10.1007/s10479-006-0045-4, p. 4
// Complexity: O((deg(v) + deg(w))^2)
bool applyAlberMainRule2(DSHunter::Instance& g, int v, int w) {
    auto N_v_without = g.neighbourhoodExcluding(v);
    auto N_w_without = g.neighbourhoodExcluding(w);
    auto N_vw_with = unite(g.neighbourhoodIncluding(v), g.neighbourhoodIncluding(w));
    auto N_vw_without = unite(N_v_without, N_w_without);

    std::vector<int> N_exit, N_guard, N_prison;
    populateExitNodes(g, N_vw_without, v, w, N_exit);

    // The only forced edges must have an end in {v, w}.
    for (auto from : N_exit) {
        for (auto [to, status] : g[from].adj) {
            if (status == DSHunter::EdgeStatus::FORCED && to != v && to != w)
                return false;
        }
    }

    populateGuardNodes(g, N_vw_without, N_exit, N_guard);
    N_prison = remove(remove(N_vw_without, N_exit), N_guard);

    auto N_prison_intersect_B = filterDominatedNodes(g, N_prison);
    if (N_prison_intersect_B.empty())
        return false;

    auto red_v = redNeighbours(g, v).size();
    auto red_w = redNeighbours(g, w).size();

    if (canBeDominatedBySingleNode(g, N_prison_intersect_B, N_guard, N_prison)) {
        return false;
    }

    bool can_be_dominated_by_just_v =
        contains(N_v_without, N_prison_intersect_B);
    bool can_be_dominated_by_just_w =
        contains(N_w_without, N_prison_intersect_B);

    DS_TRACE(std::cerr << "trying to apply " << __func__ << dbg(v) << dbg(w) << std::endl);
    if (can_be_dominated_by_just_v && can_be_dominated_by_just_w && red_v == 0 && red_w == 0) {
        return applyCase1_1(g, v, w, N_prison, N_guard, N_v_without, N_w_without);
    } 
    
    if (can_be_dominated_by_just_v && red_w == 0) {
        return applyCase1_2(g, v, N_prison, N_v_without, N_guard);
    }
    if (can_be_dominated_by_just_w && red_v == 0) {
        return applyCase1_3(g, w, N_prison, N_w_without, N_guard);
    }
    if (!can_be_dominated_by_just_v && !can_be_dominated_by_just_w) {
        return applyCase2(g, v, w, N_vw_without, N_prison, N_guard);
    }

    return false;
}
}  // namespace

namespace DSHunter {
bool alberMainRule2(Instance& g) {
    // Allocate the array once for use in breadth-first search.
    std::vector<int> dis(g.all_nodes.size(), BFS_INF);

    // Avoid rewriting the distances by considering zero to be INF - 4 * (# checked nodes),
    // since we only look at distances upto 3.
    int zero_dist = BFS_INF - 4;
    for (auto v : g.nodes) {
        if (g.isDisregarded(v)) continue;
        std::queue<int> q;
        dis[v] = zero_dist;
        q.push(v);
        while (!q.empty()) {
            int w = q.front();
            q.pop();
            if (dis[w] > zero_dist && !g.isDisregarded(w) && applyAlberMainRule2(g, v, w))
                return true;
            if (dis[w] < zero_dist + 4) {
                for (auto [x, _] : g[w].adj) {
                    if (dis[x] > dis[w] + 1) {
                        dis[x] = dis[w] + 1;
                        q.push(x);
                    }
                }
            }
        }

        zero_dist -= 4;
    }

    return false;
}

ReductionRule AlberMainRule2("AlberMainRule2", alberMainRule2, 4, 2);
}  // namespace DSHunter
