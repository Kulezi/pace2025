#ifndef RRULES_H
#define RRULES_H
#include <limits.h>

#include <functional>

#include "instance.h"
#include "utils.h"

namespace {
constexpr int BFS_INF = INT_MAX;
bool hasUndominatedNode(Instance& g, std::vector<int> nodes) {
    for (auto v : nodes)
        if (g.getNodeStatus(v) == UNDOMINATED) return true;
    return false;
}

// Checks whether node u is an exit vertex with respect to node v.
// Complexity: O(min(deg(u), deg(v)))
bool isExit(const Instance& g, int u, int v) {
    for (auto [w, _] : g.adj[u]) {
        // This will execute at most O(deg(v)) times, since g.hasEdge(v, w) can be true only
        // for deg(v) different vertices w.
        if (w != v && !g.hasEdge(v, w)) return true;
    }

    return false;
}

// Checks whether node u is an exit vertex with respect
// to the closed neighbourhood of nodes v and w.
// Complexity: O(min(deg(u), deg(v) + deg(w))
bool isExit(const Instance& g, int u, int v, int w) {
    for (auto [x, _] : g.adj[u]) {
        // This will execute at most O(deg(v) + deg(w)) times, since g.hasEdge(x, ?) can be true
        // only for deg(v) + deg(w) different vertices x.
        if (x != v && x != w && !g.hasEdge(x, v) && !g.hasEdge(x, w)) return true;
    }

    return false;
}

// Returns a sorted list of neighbours of u that have a neighour outside of the neighbourhood of u.
// Complexity: O(deg(u)^2)
std::vector<int> exitNeighbourhood(Instance& g, int u) {
    std::vector<int> N_exit;
    for (auto [v, _] : g.adj[u]) {
        if (isExit(g, v, u)) N_exit.push_back(v);
    }

    return N_exit;
}

void populateExitNodes(const Instance& g, const std::vector<int>& N_vw_without, int v, int w,
                       std::vector<int>& N_exit) {
    for (auto u : N_vw_without) {
        if (isExit(g, u, v, w)) {
            N_exit.push_back(u);
        }
    }
}

void populateGuardNodes(const Instance& g, const std::vector<int>& N_vw_without,
                        const std::vector<int>& N_exit, std::vector<int>& N_guard) {
    for (auto u : remove(N_vw_without, N_exit)) {
        auto N_u = g.neighbourhoodExcluding(u);
        if (!intersect(N_u, N_exit).empty()) {
            N_guard.push_back(u);
        }
    }
}

std::vector<int> filterDominatedNodes(const Instance& g, const std::vector<int>& N_prison) {
    std::vector<int> N_prison_intersect_B = N_prison;
    const auto new_end = std::remove_if(N_prison_intersect_B.begin(), N_prison_intersect_B.end(),
                                        [&](int u) { return g.getNodeStatus(u) != UNDOMINATED; });
    N_prison_intersect_B.erase(new_end, N_prison_intersect_B.end());
    return N_prison_intersect_B;
}

bool canBeDominatedBySingleNode(const Instance& g, const std::vector<int>& N_prison_intersect_B,
                                const std::vector<int>& N_guard, const std::vector<int>& N_prison) {
    auto intersection_can_be_dominated_by_single_from = [&](const std::vector<int>& nodes) {
        for (auto x : nodes)
            if (contains(g.neighbourhoodIncluding(x), N_prison_intersect_B)) return true;
        return false;
    };
    return N_prison_intersect_B.empty() || intersection_can_be_dominated_by_single_from(N_guard) ||
           intersection_can_be_dominated_by_single_from(N_prison);
}

void applyCase1_1(Instance& g, int v, int w, const std::vector<int>& N_prison,
                  const std::vector<int>& N_guard, const std::vector<int>& N_v_without,
                  const std::vector<int>& N_w_without) {
    int z1 = g.addNode();
    int z2 = g.addNode();
    int z3 = g.addNode();

    g.addEdge(v, z1, UNCONSTRAINED);
    g.addEdge(v, z2, UNCONSTRAINED);
    g.addEdge(v, z3, UNCONSTRAINED);

    g.addEdge(w, z1, UNCONSTRAINED);
    g.addEdge(w, z2, UNCONSTRAINED);
    g.addEdge(w, z3, UNCONSTRAINED);

    g.removeNodes(N_prison);
    g.removeNodes(intersect(intersect(N_guard, N_v_without), N_w_without));
}

void applyCase1_2(Instance& g, int v, const std::vector<int>& N_prison,
                  const std::vector<int>& N_v_without, const std::vector<int>& N_guard) {
    g.take(v);
    g.removeNodes(N_prison);
    g.removeNodes(intersect(N_v_without, N_guard));
}

void applyCase1_3(Instance& g, int w, const std::vector<int>& N_prison,
                  const std::vector<int>& N_w_without, const std::vector<int>& N_guard) {
    g.take(w);
    g.removeNode(w);
    g.removeNodes(N_prison);
    g.removeNodes(intersect(N_w_without, N_guard));
}

void applyCase2(Instance& g, int v, int w, const std::vector<int>& N_vw_without,
                const std::vector<int>& N_prison, const std::vector<int>& N_guard) {
    // w might get removed when taking v, so we need to set statuses before taking v and w.
    for (auto u : N_vw_without) {
        g.setNodeStatus(u, DOMINATED);
    }
    g.take(v);
    g.take(w);

    g.removeNodes(N_prison);
    g.removeNodes(N_guard);
}

// Tries to apply Main Rule 2 for a given pair of vertices - DOI 10.1007/s10479-006-0045-4, p. 4
// Complexity: O((deg(v) + deg(w))^2)
bool ApplyAlberMainRule2(Instance& g, int v, int w) {
    auto N_v_without = g.neighbourhoodExcluding(v);
    auto N_w_without = g.neighbourhoodExcluding(w);
    auto N_vw_with = unite(g.neighbourhoodIncluding(v), g.neighbourhoodIncluding(w));
    auto N_vw_without = unite(N_v_without, N_w_without);

    std::vector<int> N_exit, N_guard, N_prison;
    populateExitNodes(g, N_vw_without, v, w, N_exit);
    populateGuardNodes(g, N_vw_without, N_exit, N_guard);
    N_prison = remove(remove(N_vw_without, N_exit), N_guard);

    auto N_prison_intersect_B = filterDominatedNodes(g, N_prison);

    if (canBeDominatedBySingleNode(g, N_prison_intersect_B, N_guard, N_prison)) {
        return false;
    }

    bool can_be_dominated_by_just_v = contains(N_v_without, N_prison_intersect_B);
    bool can_be_dominated_by_just_w = contains(N_w_without, N_prison_intersect_B);

    if (can_be_dominated_by_just_v && can_be_dominated_by_just_w) {
        // Don't apply the reduction if it doesn't reduce the size of the graph.
        if (N_prison.size() + intersect(intersect(N_guard, N_v_without), N_w_without).size() <= 3) {
            return false;
        }
        applyCase1_1(g, v, w, N_prison, N_guard, N_v_without, N_w_without);
    } else if (can_be_dominated_by_just_v) {
        applyCase1_2(g, v, N_prison, N_v_without, N_guard);
    } else if (can_be_dominated_by_just_w) {
        applyCase1_3(g, w, N_prison, N_w_without, N_guard);
    } else {
        applyCase2(g, v, w, N_vw_without, N_prison, N_guard);
    }

    return true;
}
}  // namespace

namespace RRules {

using Rule = std::function<bool(Instance&)>;

// Naive implementation of Main Rule 1 - DOI 10.1007/s10479-006-0045-4, p. 4
// ~ O(|V|^2) or O(|V|^3) depending on the remove_node operation complexity.
bool AlberMainRule1(Instance& g) {
    for (auto u : g.nodes) {
        auto N_v_without = g.neighbourhoodExcluding(u);

        std::vector<int> N_exit = exitNeighbourhood(g, u), N_guard, N_prison;

        for (auto v : remove(N_v_without, N_exit)) {
            auto N_u = g.neighbourhoodExcluding(v);
            if (!intersect(N_u, N_exit).empty()) N_guard.push_back(v);
        }

        N_prison = remove(remove(N_v_without, N_exit), N_guard);

        if (!N_prison.empty() && hasUndominatedNode(g, N_prison)) {
            for (auto v : N_prison) g.removeNode(v);
            for (auto v : N_guard) g.removeNode(v);

            g.take(u);
            return true;
        }
    }

    return false;
}

// Naive implementation of Main Rule 2 - DOI 10.1007/s10479-006-0045-4, p. 4
// ~ O(|V|^2) or O(|V|^3) depending on the remove_node operation complexity.
bool AlberMainRule2(Instance& g) {
    // Allocate the array once for use in breadth-first search.
    std::vector<int> dis(g.next_free_id, BFS_INF);

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
            if (dis[w] > zero_dist && ApplyAlberMainRule2(g, v, w)) return true;
            if (dis[w] < zero_dist + 4) {
                for (auto [x, _] : g.adj[w]) {
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

// Batched implementation of Simple Rule 1 - DOI 10.1007/s10479-006-0045-4, p. 6
// ~ O(|E| + |V| * (# removed edges)) depending on the remove_node operation complexity.
bool AlberSimpleRule1(Instance& g) {
    std::vector<std::pair<int, int>> to_remove;
    for (auto v : g.nodes) {
        for (auto [w, status] : g.adj[v]) {
            if (v > w || status == FORCED) continue;
            if (g.getNodeStatus(v) == DOMINATED && g.getNodeStatus(w) == DOMINATED) {
                to_remove.emplace_back(v, w);
            }
        }
    }
    for (auto [v, w] : to_remove) g.removeEdge(v, w);

    return !to_remove.empty();
}

// Naive implementation of Simple Rule 2 - DOI 10.1007/s10479-006-0045-4, p. 6
// ~ O(|V| * (# removed nodes)) depending on the remove_node operation complexity.
bool AlberSimpleRule2(Instance& g) {
    std::vector<int> to_remove;
    std::vector<int> to_take;
    for (auto v : g.nodes) {
        if (g.getNodeStatus(v) == DOMINATED && g.deg(v) <= 1) {
            to_remove.push_back(v);
            if (g.deg(v) == 1) {
                auto [w, status] = g.adj[v][0];
                // The edge is forced so it's optimal to take the end that possibly could have
                // larger degree. If the other end of the edge also would be a candidate for this
                // reduction, apply it only to the vertex with smaller label.
                if (status == FORCED &&
                    !(g.deg(w) == 1 && g.getNodeStatus(w) == DOMINATED && v > w)) {
                    to_take.push_back(w);
                }
            }
        }
    }

    for (auto v : to_remove) g.removeNode(v);
    for (auto v : to_take)
        if (g.getNodeStatus(v) != TAKEN) g.take(v);
    return !to_remove.empty();
}

// Naive implementation of Simple Rule 3 - DOI 10.1007/s10479-006-0045-4, p. 6
// ~ O(|V|^2 * (# removed nodes)) depending on the remove_node operation complexity.
bool AlberSimpleRule3(Instance& g) {
    std::vector<int> to_remove;
    for (auto v : g.nodes) {
        if (g.getNodeStatus(v) != UNDOMINATED && g.deg(v) == 2) {
            int u_1 = g.adj[v].front().to;
            int u_2 = g.adj[v][1].to;
            if (g.getNodeStatus(u_1) == UNDOMINATED && g.getNodeStatus(u_2) == UNDOMINATED) {
                if (g.hasEdge(u_1, u_2)) {
                    // 3.1
                    g.removeNode(v);
                    return true;
                } else {
                    // 3.2
                    for (auto [u, _] : g.adj[u_1]) {
                        if (u == v) continue;
                        if (g.hasEdge(u, u_2)) {
                            g.removeNode(v);
                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}

// Naive implementation of Simple Rule 4 - DOI 10.1007/s10479-006-0045-4, p. 6
// ~ O(|V| * (# removed nodes)) depending on the remove_node operation complexity.
bool AlberSimpleRule4(Instance& g) {
    std::vector<int> to_remove;
    for (auto v : g.nodes) {
        if (g.getNodeStatus(v) != UNDOMINATED && g.deg(v) == 3) {
            auto it = g.adj[v].begin();
            int u_1 = it->to;
            int u_2 = (++it)->to;
            int u_3 = (++it)->to;

            if (g.getNodeStatus(u_1) == UNDOMINATED && g.getNodeStatus(u_2) == UNDOMINATED &&
                g.getNodeStatus(u_3) == UNDOMINATED) {
                int n_edges = (g.hasEdge(u_1, u_2) + g.hasEdge(u_2, u_3) + g.hasEdge(u_1, u_3));
                if (n_edges >= 2) {
                    to_remove.push_back(v);
                }
            }
        }
    }

    for (auto v : to_remove) g.removeNode(v);
    return !to_remove.empty();
}

void forceEdge(Instance& g, int u, int v) {
    DS_ASSERT(g.hasEdge(u, v));
    DS_ASSERT(g.getEdgeStatus(u, v) != FORCED);
    g.setEdgeStatus(u, v, FORCED);
    g.setNodeStatus(u, DOMINATED);
    g.setNodeStatus(v, DOMINATED);

    // All vertices that see both endpoints of this edge must be dominated by one of them,
    // so we can mark them as dominated.
    auto edgeNeighbourhood = intersect(g.neighbourhoodExcluding(u), g.neighbourhoodExcluding(v));
    for (auto w : edgeNeighbourhood) {
        g.setNodeStatus(w, DOMINATED);
    }
}

// If a vertex of degree two is contained in the neighbourhoods of both its neighbours,
// and they are connected by an edge, make the edge forced and remove this vertex, as there exists
// an optimal solution not-taking this vertex and taking one of its neighbours.
// Complexity: ~ O(|V| * (# removed nodes)) depending on the remove_node operation complexity.
bool ForcedEdgeRule(Instance& g) {
    auto nodes = g.nodes;
    for (auto v : nodes) {
        if (g.deg(v) == 2 && g.getNodeStatus(v) == UNDOMINATED) {
            auto e1 = g.adj[v][0];
            auto e2 = g.adj[v][1];
            if (!g.hasEdge(e1.to, e2.to)) continue;

            if (e1.status == UNCONSTRAINED && e2.status == UNCONSTRAINED) {
                forceEdge(g, e1.to, e2.to);
                g.removeNode(v);
                return true;
            } else if (e1.status == FORCED && e2.status == UNCONSTRAINED) {
                // Taking e1.to is optimal, as it's always better than taking v, and we are forced
                // to take one of them.
                g.take(e1.to);
                return true;
            } else if (e1.status == UNCONSTRAINED && e2.status == FORCED) {
                // Taking e2.to is optimal, as it's always better than taking v, and we are forced
                // to take one of them.
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

const std::vector<RRules::Rule> defaults_preprocess = {
    RRules::AlberSimpleRule1, RRules::AlberSimpleRule2, RRules::AlberSimpleRule3,
    RRules::AlberSimpleRule4, RRules::AlberMainRule1,   RRules::AlberMainRule2,
};

const std::vector<RRules::Rule> defaults_branching = {
    RRules::AlberSimpleRule1, RRules::AlberSimpleRule2, RRules::AlberSimpleRule3,
    RRules::AlberSimpleRule4, RRules::AlberMainRule1,
};

}  // namespace RRules

#endif  // RRULES_H
