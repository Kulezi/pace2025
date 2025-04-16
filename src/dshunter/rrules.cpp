#include "rrules.h"

#include <limits.h>

#include <functional>
#include <queue>
#include <vector>

#include "utils.h"
namespace {
using DSHunter::intersect, DSHunter::contains, DSHunter::unite, DSHunter::remove;
constexpr int BFS_INF = INT_MAX;
bool hasUndominatedNode(DSHunter::Instance& g, std::vector<int> nodes) {
    for (auto v : nodes)
        if (!g.isDominated(v)) return true;
    return false;
}

// Checks whether node u is an exit vertex with respect to node v.
// Complexity: O(min(deg(u), deg(v)))
bool isExit(const DSHunter::Instance& g, int u, int v) {
    for (auto [w, status] : g[u].adj) {
        // This will execute at most O(deg(v)) times, since g.hasEdge(v, w) can be true only
        // for deg(v) different vertices w.
        if (w != v && (!g.hasEdge(v, w) || status == DSHunter::EdgeStatus::FORCED)) return true;
    }

    return false;
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

// Returns a sorted list of neighbours of u that have a neighour outside of the neighbourhood of u.
// Complexity: O(deg(u)^2)
std::vector<int> exitNeighbourhood(DSHunter::Instance& g, int u) {
    std::vector<int> N_exit;
    for (auto [v, _] : g[u].adj) {
        if (isExit(g, v, u)) N_exit.push_back(v);
    }

    return N_exit;
}

void populateExitNodes(const DSHunter::Instance& g, const std::vector<int>& N_vw_without, int v,
                       int w, std::vector<int>& N_exit) {
    for (auto u : N_vw_without) {
        if (isExit(g, u, v, w)) {
            N_exit.push_back(u);
        }
    }
}

void populateGuardNodes(const DSHunter::Instance& g, const std::vector<int>& N_vw_without,
                        const std::vector<int>& N_exit, std::vector<int>& N_guard) {
    for (auto u : remove(N_vw_without, N_exit)) {
        auto N_u = g.neighbourhoodExcluding(u);
        if (!intersect(N_u, N_exit).empty()) {
            N_guard.push_back(u);
        }
    }
}

std::vector<int> filterDominatedNodes(const DSHunter::Instance& g,
                                      const std::vector<int>& N_prison) {
    std::vector<int> N_prison_intersect_B = N_prison;
    const auto new_end = std::remove_if(N_prison_intersect_B.begin(), N_prison_intersect_B.end(),
                                        [&](int u) { return g.isDominated(u); });
    N_prison_intersect_B.erase(new_end, N_prison_intersect_B.end());
    return N_prison_intersect_B;
}

bool canBeDominatedBySingleNode(const DSHunter::Instance& g,
                                const std::vector<int>& N_prison_intersect_B,
                                const std::vector<int>& N_guard, const std::vector<int>& N_prison) {
    auto intersection_can_be_dominated_by_single_from = [&](const std::vector<int>& nodes) {
        for (auto x : nodes)
            if (contains(g.neighbourhoodIncluding(x), N_prison_intersect_B)) return true;
        return false;
    };
    return N_prison_intersect_B.empty() || intersection_can_be_dominated_by_single_from(N_guard) ||
           intersection_can_be_dominated_by_single_from(N_prison);
}

bool applyCase1_1(DSHunter::Instance& g, int v, int w, const std::vector<int>& N_prison,
                  const std::vector<int>& N_guard, const std::vector<int>& N_v_without,
                  const std::vector<int>& N_w_without) {
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

bool applyCase1_2(DSHunter::Instance& g, int v, const std::vector<int>& N_prison,
                  const std::vector<int>& N_v_without, const std::vector<int>& N_guard) {
    DS_TRACE(std::cerr << "applying " << __func__ << dbg(v) << dbgv(N_prison) << dbgv(N_guard)
                       << dbgv(N_v_without) << std::endl);
    g.take(v);
    g.removeNodes(N_prison);
    g.removeNodes(intersect(N_v_without, N_guard));
    return true;
}

bool applyCase1_3(DSHunter::Instance& g, int w, const std::vector<int>& N_prison,
                  const std::vector<int>& N_w_without, const std::vector<int>& N_guard) {
    DS_TRACE(std::cerr << "applying " << __func__ << dbg(w) << dbgv(N_prison) << dbgv(N_guard)
                       << dbgv(N_w_without) << std::endl);
    g.take(w);
    g.removeNode(w);
    g.removeNodes(N_prison);
    g.removeNodes(intersect(N_w_without, N_guard));
    return true;
}

bool applyCase2(DSHunter::Instance& g, int v, int w, const std::vector<int>& N_vw_without,
                const std::vector<int>& N_prison, const std::vector<int>& N_guard) {
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

std::vector<int> redNeighbours(DSHunter::Instance& g, int v) {
    std::vector<int> res;
    for (auto [u, status] : g[v].adj)
        if (status == DSHunter::EdgeStatus::FORCED) res.push_back(u);
    return res;
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
            if (status == DSHunter::EdgeStatus::FORCED && to != v && to != w) return false;
        }
    }

    populateGuardNodes(g, N_vw_without, N_exit, N_guard);
    N_prison = remove(remove(N_vw_without, N_exit), N_guard);

    auto N_prison_intersect_B = filterDominatedNodes(g, N_prison);
    if (N_prison_intersect_B.empty()) return false;

    auto red_v = redNeighbours(g, v);
    auto red_w = redNeighbours(g, w);

    if (canBeDominatedBySingleNode(g, N_prison_intersect_B, N_guard, N_prison)) {
        return false;
    }

    bool can_be_dominated_by_just_v =
        contains(N_v_without, N_prison_intersect_B) && red_w.size() == 0;
    bool can_be_dominated_by_just_w =
        contains(N_w_without, N_prison_intersect_B) && red_v.size() == 0;

    DS_TRACE(std::cerr << "trying to apply " << __func__ << dbg(v) << dbg(w) << std::endl);
    if (can_be_dominated_by_just_v && can_be_dominated_by_just_w) {
        return applyCase1_1(g, v, w, N_prison, N_guard, N_v_without, N_w_without);
    }
    if (can_be_dominated_by_just_v) {
        return applyCase1_2(g, v, N_prison, N_v_without, N_guard);
    }
    if (can_be_dominated_by_just_w) {
        return applyCase1_3(g, w, N_prison, N_w_without, N_guard);
    }
    return applyCase2(g, v, w, N_vw_without, N_prison, N_guard);
}

bool haveCommonNeighbour(const DSHunter::Instance& g, int u, int v) {
    for (auto [w, _] : g[u].adj) {
        if (v == w) continue;
        if (g.hasEdge(v, v)) return true;
    }
    return false;
}

bool tryMidpoint(DSHunter::Instance& g, bool forced_by_edge, int u, int v, int w) {
    if (g.hasEdge(u, v) && g.hasEdge(u, w)) {
        if (forced_by_edge) {
            DS_TRACE(std::cerr << "applying " << __func__ << "(take)" << dbg(u) << std::endl);
            g.take(u);
        }
        return true;
    }

    return false;
}

}  // namespace

namespace DSHunter {
bool ReductionRule::apply(Instance& g) const {
    DS_TRACE(std::cerr << "trying to apply " << name << " (n=" << g.nodeCount() << ", m="
                       << g.edgeCount() << ", f=" << g.forcedEdgeCount() << ")" << std::endl);

    bool applied = f(g);

    DS_TRACE(if (applied) std::cerr << "succesfully applied " << name << " (n=" << g.nodeCount()
                                    << ", m=" << g.edgeCount() << ", f=" << g.forcedEdgeCount()
                                    << ")" << std::endl;
             else std::cerr << "failed to apply " << name << std::endl;);
    return applied;
}

void reduce(Instance& g, std::vector<ReductionRule>& reduction_rules, int complexity) {
_start:
    for (auto& rule : reduction_rules) {
        if (rule.complexity_dense > complexity) continue;
        bool reduced = rule.apply(g);
        ++rule.application_count;

        if (reduced) {
            ++rule.success_count;
            goto _start;
        }
    }
}

bool alberMainRule1(Instance& g) {
    for (auto u : g.nodes) {
        auto N_v_without = g.neighbourhoodExcluding(u);

        std::vector<int> N_exit = exitNeighbourhood(g, u), N_guard, N_prison;

        for (auto v : remove(N_v_without, N_exit)) {
            auto N_u = g.neighbourhoodExcluding(v);
            if (!intersect(N_u, N_exit).empty()) N_guard.push_back(v);
        }

        N_prison = remove(remove(N_v_without, N_exit), N_guard);

        if (!N_prison.empty() && hasUndominatedNode(g, N_prison)) {
            DS_TRACE(std::cerr << "applying " << __func__ << dbg(u) << dbgv(N_prison)
                               << dbgv(N_guard) << dbgv(N_exit) << std::endl);
            g.take(u);

            for (auto v : N_prison) g.removeNode(v);
            for (auto v : N_guard) g.removeNode(v);
            return true;
        }
    }

    return false;
}

bool alberMainRule2(Instance& g) {
    // Allocate the array once for use in breadth-first search.
    std::vector<int> dis(g.all_nodes.size(), BFS_INF);

    // Avoid rewriting the distances by considering zero to be INF - 4 * (# checked nodes),
    // since we only look at distances upto 3.
    int zero_dist = BFS_INF - 4;
    for (auto v : g.nodes) {
        std::queue<int> q;
        dis[v] = zero_dist;
        q.push(v);
        while (!q.empty()) {
            int w = q.front();
            q.pop();
            if (dis[w] > zero_dist && applyAlberMainRule2(g, v, w)) return true;
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

bool alberSimpleRule1(Instance& g) {
    std::vector<std::pair<int, int>> to_remove;
    for (auto v : g.nodes) {
        for (auto [w, status] : g[v].adj) {
            if (v > w || status == EdgeStatus::FORCED) continue;
            if (g.isDominated(v) && g.isDominated(w)) {
                to_remove.emplace_back(v, w);
            }
        }
    }
    for (auto [v, w] : to_remove) {
        DS_TRACE(std::cerr << "applying " << __func__ << dbg(v) << dbg(w) << std::endl);
        g.removeEdge(v, w);
    }

    return !to_remove.empty();
}

bool alberSimpleRule2(Instance& g) {
    std::vector<int> to_remove;
    std::vector<int> to_take;
    for (auto v : g.nodes) {
        if (g.isDominated(v) && g.deg(v) <= 1) {
            to_remove.push_back(v);
            if (g.deg(v) == 1) {
                auto [w, status] = g[v].adj[0];
                // The edge is forced so it's optimal to take the end that possibly could have
                // larger degree. If the other end of the edge also would be a candidate for
                // this reduction, apply it only to the vertex with smaller label.
                if (status == EdgeStatus::FORCED && !(g.deg(w) == 1 && g.isDominated(w) && v > w)) {
                    to_take.push_back(w);
                }
            }
        }
    }

    for (auto v : to_take) {
        if (!g.isTaken(v)) {
            DS_TRACE(std::cerr << "applying " << __func__ << " (take) " << dbg(v) << std::endl);
            g.take(v);
        }
    }

    for (auto v : to_remove) {
        DS_TRACE(std::cerr << "applying " << __func__ << " (remove) " << dbg(v) << std::endl);
        g.removeNode(v);
    }

    return !to_remove.empty() || !to_take.empty();
}

bool alberSimpleRule3(Instance& g) {
    for (auto v : g.nodes) {
        if (g.isDominated(v) && g.deg(v) == 2) {
            auto [u_1, s_1] = g[v].adj.front();
            auto [u_2, s_2] = g[v].adj[1];
            // In this case it actually might be optimal to take this vertex instead of the two.
            if (s_1 == EdgeStatus::FORCED && s_2 == EdgeStatus::FORCED) continue;
            bool should_remove = !g.isDominated(u_1) && !g.isDominated(u_2) &&
                                 (g.hasEdge(u_1, u_2) || haveCommonNeighbour(g, u_1, u_2));

            if (should_remove) {
                DS_TRACE(std::cerr << "applying " << __func__ << dbg(v) << std::endl);

                DS_ASSERT(s_1 != EdgeStatus::FORCED || s_2 != EdgeStatus::FORCED);
                if (s_1 == EdgeStatus::FORCED) g.take(u_1);
                if (s_2 == EdgeStatus::FORCED) g.take(u_2);

                g.removeNode(v);

                return true;
            }
        }
    }

    return false;
}

bool alberSimpleRule4(Instance& g) {
    for (auto v : g.nodes) {
        if (g.isDominated(v) && g.deg(v) == 3) {
            auto [u_1, s_1] = g[v].adj[0];
            auto [u_2, s_2] = g[v].adj[1];
            auto [u_3, s_3] = g[v].adj[2];

            int n_forced_edges = (int)s_1 + (int)s_2 + (int)s_3;
            // There can be at most one forced edge, and it needs to lead to a vertex that can
            // dominate all three others.
            bool possibly_valid = !g.isDominated(u_1) && !g.isDominated(u_2) &&
                                  !g.isDominated(u_3) && n_forced_edges <= 1;

            if (possibly_valid) {
                if (tryMidpoint(g, (bool)s_1, u_1, u_2, u_3) ||
                    tryMidpoint(g, (bool)s_2, u_2, u_1, u_3) ||
                    tryMidpoint(g, (bool)s_3, u_3, u_1, u_2)) {
                    DS_TRACE(std::cerr << "applying " << __func__ << dbg(v) << std::endl);
                    g.removeNode(v);
                    return true;
                }
            }
        }
    }

    return false;
}

bool forcedEdgeRule(Instance& g) {
    auto nodes = g.nodes;
    for (auto v : nodes) {
        if (g.deg(v) == 2 && !g.isDominated(v)) {
            auto e1 = g[v].adj[0];
            auto e2 = g[v].adj[1];
            if (!g.hasEdge(e1.to, e2.to)) continue;

            if (e1.status == EdgeStatus::UNCONSTRAINED && e2.status == EdgeStatus::UNCONSTRAINED) {
                DS_TRACE(std::cerr << __func__ << "(1)" << dbg(v) << std::endl);
                g.removeNode(v);
                if (g.getEdgeStatus(e1.to, e2.to) != EdgeStatus::FORCED) g.forceEdge(e1.to, e2.to);
                return true;
            } else if (e1.status == EdgeStatus::FORCED && e2.status == EdgeStatus::UNCONSTRAINED) {
                DS_TRACE(std::cerr << __func__ << "(2)" << dbg(v) << dbg(e1.to) << std::endl);
                // Taking e1.to is optimal, as it's always better than taking v, and we are
                // forced to take one of them.
                g.take(e1.to);

                return true;
            } else if (e1.status == EdgeStatus::UNCONSTRAINED && e2.status == EdgeStatus::FORCED) {
                DS_TRACE(std::cerr << __func__ << "(3)" << dbg(v) << dbg(e2.to) << std::endl);
                // Taking e2.to is optimal, as it's always better than taking v, and we are
                // forced to take one of them.
                g.take(e2.to);

                return true;
            } else {
                // We need to take either just u, or both e1.to and e2.to.
                // TODO: contract them into a single vertex thats treated the following way:
                // If it's taken into ds it adds e1.to and e2.to to the dominating set.
                // If it's not taken it adds just v to the dominating set.
            }
        }
    }

    return false;
}

ReductionRule AlberMainRule1("AlberMainRule1", alberMainRule1, 3, 1);
ReductionRule AlberMainRule2("AlberMainRule2", alberMainRule2, 4, 2);
ReductionRule AlberSimpleRule1("AlberSimpleRule1 (dominated edge removal)", alberSimpleRule1, 2, 1);
ReductionRule AlberSimpleRule2("AlberSimpleRule2 (dominated leaf removal)", alberSimpleRule2, 2, 1);
ReductionRule AlberSimpleRule3("AlberSimpleRule3 (dominated degree 2 vertex removal)",
                               alberSimpleRule3, 2, 1);
ReductionRule AlberSimpleRule4("AlberSimpleRule4 (dominated degree 3 vertex removal)",
                               alberSimpleRule4, 2, 1);
ReductionRule ForcedEdgeRule("ForcedEdgeRule", forcedEdgeRule, 1, 1);

const std::vector<ReductionRule> default_reduction_rules = {

    // Then rules that don't affect the graph structure.
    ForcedEdgeRule,

    // First the cheap rules that only remove vertices.
    AlberSimpleRule1, AlberSimpleRule2, AlberSimpleRule3, AlberSimpleRule4,

    // Then more expensive rules.
    AlberMainRule1, AlberMainRule2};
}  // namespace DSHunter
