#include <climits>
#include <queue>

#include "../rrules.h"

namespace {
using DSHunter::intersect, DSHunter::contains, DSHunter::unite, DSHunter::remove, DSHunter::Endpoint, DSHunter::Instance;
using std::vector;
constexpr int BFS_INF = INT_MAX;

bool applyCase1_1(Instance& g, const int v, const int w, const vector<int>& N_prison, const vector<int>& N_guard, const vector<int>& N_v_without, const vector<int>& N_w_without) {
    // Calculate intersection before we invalidate our references.
    const auto removable_intersection = intersect(intersect(N_guard, N_v_without), N_w_without);
    if (!g.hasEdge(v, w)) {
        // Don't apply the reduction if it doesn't reduce the size of the graph.
        if (N_prison.size() + removable_intersection.size() <= 3)
            return false;
        DS_TRACE(std::cerr << "applying " << __func__ << " (with gadget vertex) " << dbg(v)
                           << dbg(w) << dbgv(N_prison) << dbgv(N_guard) << dbgv(N_v_without)
                           << dbgv(N_w_without) << std::endl);
        const int z1 = g.addNode();
        const int z2 = g.addNode();
        const int z3 = g.addNode();

        g.addEdge(v, z1);
        g.addEdge(v, z2);
        g.addEdge(v, z3);

        g.addEdge(w, z1);
        g.addEdge(w, z2);
        g.addEdge(w, z3);

        g.removeNodes(N_prison);
        g.removeNodes(removable_intersection);
        return true;
    }
    DS_TRACE(std::cerr << "applying " << __func__ << " (edge forcing) " << dbg(v) << dbg(w)
                       << dbgv(N_prison) << dbgv(N_guard) << dbgv(N_v_without) << dbgv(N_w_without)
                       << std::endl);
    // The edge was already there but could be forced already.
    if (g.getEdgeStatus(v, w) == DSHunter::EdgeStatus::UNCONSTRAINED) {
        g.forceEdge(v, w);
    }
    g.removeNodes(N_prison);
    g.removeNodes(removable_intersection);

    return true;
}

bool applyCase1_2(Instance& g, const int v, const vector<int>& N_prison, const vector<int>& N_v_without, const vector<int>& N_guard) {
    DS_TRACE(std::cerr << "applying " << __func__ << dbg(v) << dbgv(N_prison) << dbgv(N_guard)
                       << dbgv(N_v_without) << std::endl);
    const auto removable_intersection = intersect(N_v_without, N_guard);
    g.take(v);
    g.removeNodes(N_prison);
    g.removeNodes(removable_intersection);
    return true;
}

bool applyCase1_3(Instance& g, const int w, const vector<int>& N_prison, const vector<int>& N_w_without, const vector<int>& N_guard) {
    DS_TRACE(std::cerr << "applying " << __func__ << dbg(w) << dbgv(N_prison) << dbgv(N_guard)
                       << dbgv(N_w_without) << std::endl);
    const auto removable_intersection = intersect(N_w_without, N_guard);
    g.take(w);
    g.removeNodes(N_prison);
    g.removeNodes(removable_intersection);
    return true;
}

bool applyCase2(Instance& g, int v, int w, const vector<int>& N_vw_without, const vector<int>& N_prison, const vector<int>& N_guard) {
    DS_TRACE(std::cerr << "applying " << __func__ << dbg(v) << dbg(w) << dbgv(N_prison)
                       << dbgv(N_guard) << std::endl);
    // w might get removed when taking v, so we need to set statuses before taking v and w.
    for (const auto u : N_vw_without) {
        g.markDominated(u);
    }
    g.take(v);
    g.take(w);

    g.removeNodes(N_prison);
    g.removeNodes(N_guard);

    return true;
}

// Checks whether node u is an exit vertex with respect
// to the closed neighborhood of nodes v and w.
// Vertices with a red edge pointing to somewhere else than v and w
// are considered exit vertices. Edge like that means there was an additional vertex
// that would be reachable outside the neighborhood, hence we would be able to exit.
// Complexity: O(min(deg(u), deg(v) + deg(w))
bool isExit(const Instance& g, const int u, const int v, const int w) {
    return std::ranges::any_of(g[u].adj, [&](const Endpoint e) {
            // This will execute at most O(deg(v) + deg(w)) times, since g.hasEdge(x, ?) can be true
            // only for deg(v) + deg(w) different vertices x.
            return e.to != v && e.to != w &&
                    ((!g.hasEdge(e.to, v) && !g.hasEdge(e.to, w)) || e.status == DSHunter::EdgeStatus::FORCED);
        });
}

void populateExitNodes(const Instance& g, const vector<int>& N_vw_without, int v, int w, vector<int>& N_exit) {
        for (auto u : N_vw_without) {
            if (isExit(g, u, v, w)) {
                N_exit.push_back(u);
            }
        }
}

void populateGuardNodes(const Instance& g, const vector<int>& N_vw_without, const vector<int>& N_exit, vector<int>& N_guard) {
        for (auto u : remove(N_vw_without, N_exit)) {
            if (!intersect(g[u].n_open, N_exit).empty()) {
                N_guard.push_back(u);
            }
        }
}

vector<int> redNeighbours(const Instance& g, const int v) {
        vector<int> res;
        for (auto [u, status] : g[v].adj)
            if (status == DSHunter::EdgeStatus::FORCED)
                res.push_back(u);
        return res;
}

vector<int> filterDominatedNodes(const Instance& g,
                                      const vector<int>& N_prison) {
        vector<int> N_prison_intersect_B = N_prison;
        const auto new_end = std::ranges::remove_if(N_prison_intersect_B, [&](int u) { return g.isDominated(u); }).begin();
        N_prison_intersect_B.erase(new_end, N_prison_intersect_B.end());
        return N_prison_intersect_B;
}

bool canBeDominatedBySingleNode(const Instance& g,
                                const vector<int>& N_prison_intersect_B,
                                const vector<int>& N_guard,
                                const vector<int>& N_prison) {
        auto intersection_can_be_dominated_by_single_from = [&](const vector<int>& nodes) {
            return std::ranges::any_of(nodes, [&](int v) {return contains(g[v].n_closed, N_prison_intersect_B);});
        };
        return N_prison_intersect_B.empty() || intersection_can_be_dominated_by_single_from(N_guard) ||
               intersection_can_be_dominated_by_single_from(N_prison);
}

// Tries to apply Main Rule 2 for a given pair of vertices - DOI 10.1007/s10479-006-0045-4, p. 4
// Complexity: O((deg(v) + deg(w))^2)
bool applyAlberMainRule2(Instance& g, const int v, const int w) {
        auto N_vw_with = unite(g[v].n_closed, g[w].n_closed);
    const auto N_vw_without = unite(g[v].n_open, g[w].n_open);

        vector<int> N_exit, N_guard;
        populateExitNodes(g, N_vw_without, v, w, N_exit);

        // The only forced edges must have an end in {v, w}.
        for (const auto from : N_exit) {
            for (auto [to, status] : g[from].adj) {
                if (status == DSHunter::EdgeStatus::FORCED && to != v && to != w)
                    return false;
            }
        }

        populateGuardNodes(g, N_vw_without, N_exit, N_guard);
        const vector<int> N_prison = remove(remove(N_vw_without, N_exit), N_guard);

        const auto N_prison_intersect_B = filterDominatedNodes(g, N_prison);
        if (N_prison_intersect_B.empty())
            return false;

        const auto red_v = redNeighbours(g, v).size();
        const auto red_w = redNeighbours(g, w).size();

        if (canBeDominatedBySingleNode(g, N_prison_intersect_B, N_guard, N_prison)) {
            return false;
        }

        const bool can_be_dominated_by_just_v =
            contains(g[v].n_open, N_prison_intersect_B);
        const bool can_be_dominated_by_just_w =
            contains(g[w].n_open, N_prison_intersect_B);

        DS_TRACE(std::cerr << "trying to apply " << __func__ << dbg(v) << dbg(w) << std::endl);
        if (can_be_dominated_by_just_v && can_be_dominated_by_just_w && red_v == 0 && red_w == 0) {
            return applyCase1_1(g, v, w, N_prison, N_guard, g[v].n_open, g[w].n_open);
        }

        if (can_be_dominated_by_just_v && !can_be_dominated_by_just_w && red_w == 0) {
            return applyCase1_2(g, v, N_prison, g[v].n_open, N_guard);
        }
        if (!can_be_dominated_by_just_v && can_be_dominated_by_just_w && red_v == 0) {
            return applyCase1_3(g, w, N_prison, g[w].n_open, N_guard);
        }
        if (!can_be_dominated_by_just_v && !can_be_dominated_by_just_w) {
            return applyCase2(g, v, w, N_vw_without, N_prison, N_guard);
        }

        return false;
}
}  // namespace

namespace DSHunter {
bool alberMainRule2(Instance& g) {
    const auto nodes = g.nodes;
    bool reduced = false;

    // Allocate the array once for use in breadth-first search.
    vector<int> dis(g.all_nodes.size(), BFS_INF);

    // Avoid rewriting the distances by considering zero to be INF - 4 * (# checked nodes),
    // since we only look at distances up to 3.
    int zero_dist = BFS_INF - 4;
    for (auto v : nodes) {
        if (!g.hasNode(v) || g.isDisregarded(v))
            continue;
        std::queue<int> q;
        dis[v] = zero_dist;
        q.push(v);
        while (!q.empty()) {
            const int w = q.front();
            q.pop();
            if (dis[w] > zero_dist && !g.isDisregarded(w) && applyAlberMainRule2(g, v, w)) {
                reduced = true;
                // We might've removed node v from the graph, so we break out of the while loop.
                // We can still look for new pairs after v.
                break;
            }
            if (dis[w] < zero_dist + 4) {
                for (auto x : g[w].n_open) {
                    if (dis[x] > dis[w] + 1) {
                        dis[x] = dis[w] + 1;
                        q.push(x);
                    }
                }
            }
        }

        zero_dist -= 4;
    }

    return reduced;
}

ReductionRule AlberMainRule2("AlberMainRule2", alberMainRule2, 4, 2);
}  // namespace DSHunter
