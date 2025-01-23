#ifndef RRULES_H
#define RRULES_H
#include <functional>

#include "instance.h"
#include "setops.h"
namespace RRules {

using Rule = std::function<bool(Instance &)>;

void reduce(Instance &g, std::vector<Rule> rules) {
_start:
    for (size_t i = 0; i < rules.size(); i++) {
        auto f = rules[i];
        bool reduced = f(g);
        if (reduced) goto _start;
    }
}

bool hasUndominatedNode(Instance &g, std::vector<int> nodes) {
    for (auto v : nodes)
        if (g.getStatus(v) == UNDOMINATED) return true;
    return false;
}

// Naive implementation of Main Rule 1 - DOI 10.1007/s10479-006-0045-4, p. 4
// ~ O(|V|^2) or O(|V|^3) depending on the remove_node operation complexity.
bool AlberMainRule1(Instance &g) {
    for (auto v : g.nodes) {
        auto N_v_with = g.neighbourhoodIncluding(v);
        auto N_v_without = g.neighbourhoodExcluding(v);

        std::vector<int> N_exit, N_guard, N_prison;
        for (auto u : N_v_without) {
            auto N_u = g.neighbourhoodExcluding(u);
            if (!remove(N_u, N_v_with).empty()) N_exit.push_back(u);
        }

        for (auto u : remove(N_v_without, N_exit)) {
            auto N_u = g.neighbourhoodExcluding(u);
            if (!intersect(N_u, N_exit).empty()) N_guard.push_back(u);
        }

        N_prison = remove(remove(N_v_without, N_exit), N_guard);

        if (!N_prison.empty() && hasUndominatedNode(g, N_prison)) {
            for (auto u : N_prison) g.removeNode(u);
            for (auto u : N_guard) g.removeNode(u);

            g.take(v);
            return true;
        }
    }

    return false;
}

// Naive implementation of Main Rule 2 - DOI 10.1007/s10479-006-0045-4, p. 4
// ~ O(|V|^2) or O(|V|^3) depending on the remove_node operation complexity.
bool AlberMainRule2(Instance &g) {
    for (auto v : g.nodes) {
        for (auto w : g.nodes) {
            if (v == w) continue;
            auto N_v_without = g.neighbourhoodExcluding(v);
            auto N_w_without = g.neighbourhoodExcluding(w);

            auto N_vw_with = unite(g.neighbourhoodIncluding(v), g.neighbourhoodIncluding(w));
            auto N_vw_without = unite(N_v_without, N_w_without);

            std::vector<int> N_exit, N_guard, N_prison;
            for (auto u : N_vw_without) {
                auto N_u = g.neighbourhoodExcluding(u);

                if (!remove(N_u, N_vw_with).empty()) N_exit.push_back(u);
            }

            for (auto u : remove(N_vw_without, N_exit)) {
                auto N_u = g.neighbourhoodExcluding(u);
                if (!intersect(N_u, N_exit).empty()) N_guard.push_back(u);
            }

            N_prison = remove(remove(N_vw_without, N_exit), N_guard);
            auto N_prison_intersect_B = N_prison;
            const auto new_end =
                std::remove_if(N_prison_intersect_B.begin(), N_prison_intersect_B.end(),
                               [&](int u) { return g.getStatus(u) != UNDOMINATED; });
            N_prison_intersect_B.erase(new_end, N_prison_intersect_B.end());

            auto intersection_can_be_dominated_by_single_from = [&](std::vector<int> &nodes) {
                for (auto x : nodes)
                    if (contains(g.neighbourhoodIncluding(x), N_prison_intersect_B)) return true;
                return false;
            };

            if (!N_prison_intersect_B.empty() &&
                !intersection_can_be_dominated_by_single_from(N_guard) &&
                !intersection_can_be_dominated_by_single_from(N_prison)) {
                bool can_be_dominated_by_just_v = contains(N_v_without, N_prison_intersect_B);
                bool can_be_dominated_by_just_w = contains(N_w_without, N_prison_intersect_B);

                // TODO: Maybe it's better to annotate that the one of {v, w} needs to be in the
                // domset.
                if (can_be_dominated_by_just_v && can_be_dominated_by_just_w) {
                    // Don't apply the reduction if it doesn't reduce the size of the graph
                    if (N_prison.size() +
                            intersect(intersect(N_guard, N_v_without), N_w_without).size() <=
                        3)
                        continue;
                    // Case 1.1
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
                } else if (can_be_dominated_by_just_v) {
                    // Case 1.2
                    g.take(v);
                    g.removeNodes(N_prison);
                    g.removeNodes(intersect(N_v_without, N_guard));
                } else if (can_be_dominated_by_just_w) {
                    // Case 1.3
                    g.take(w);
                    // TODO: Order of those operations can be changed to reduce reduntant ops.
                    g.removeNode(w);
                    g.removeNodes(N_prison);
                    g.removeNodes(intersect(N_w_without, N_guard));
                } else {
                    // Case 2
                    // w might get removed when taking v, so we need to set statuses before taking v
                    // and w.
                    for (auto u : N_vw_without) g.setStatus(u, DOMINATED);
                    g.take(v);
                    g.take(w);

                    g.removeNodes(N_prison);
                    g.removeNodes(N_guard);
                }

                return true;
            }
        }
    }

    return false;
}

// Naive implementation of Simple Rule 1 - DOI 10.1007/s10479-006-0045-4, p. 6
// ~ O(|E| + |V| * (# removed edges)) depending on the remove_node operation complexity.
bool AlberSimpleRule1(Instance &g) {
    std::vector<std::pair<int, int>> to_remove;
    for (auto v : g.nodes) {
        for (auto w : g.adj[v]) {
            if (v > w) continue;
            if (g.getStatus(v) == DOMINATED && g.getStatus(w) == DOMINATED) {
                to_remove.emplace_back(v, w);
            }
        }
    }
    for (auto [v, w] : to_remove) g.removeEdge(v, w);

    return !to_remove.empty();
}

// Naive implementation of Simple Rule 2 - DOI 10.1007/s10479-006-0045-4, p. 6
// ~ O(|V| * (# removed nodes)) depending on the remove_node operation complexity.
bool AlberSimpleRule2(Instance &g) {
    std::vector<int> to_remove;
    for (auto v : g.nodes) {
        if (g.getStatus(v) == DOMINATED && g.deg(v) <= 1) {
            to_remove.push_back(v);
        }
    }

    for (auto v : to_remove) g.removeNode(v);
    return !to_remove.empty();
}

// Naive implementation of Simple Rule 3 - DOI 10.1007/s10479-006-0045-4, p. 6
// ~ O(|V|^2 * (# removed nodes)) depending on the remove_node operation complexity.
bool AlberSimpleRule3(Instance &g) {
    std::vector<int> to_remove;
    for (auto v : g.nodes) {
        if (g.getStatus(v) != UNDOMINATED && g.deg(v) == 2) {
            int u_1 = g.adj[v].front();
            int u_2 = g.adj[v][1];
            if (g.getStatus(u_1) == UNDOMINATED && g.getStatus(u_2) == UNDOMINATED) {
                if (g.hasEdge(u_1, u_2)) {
                    // 3.1
                    g.removeNode(v);
                    return true;
                } else {
                    // 3.2
                    for (auto u : g.adj[u_1]) {
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
bool AlberSimpleRule4(Instance &g) {
    std::vector<int> to_remove;
    for (auto v : g.nodes) {
        if (g.getStatus(v) != UNDOMINATED && g.deg(v) == 3) {
            auto it = g.adj[v].begin();
            int u_1 = *it;
            int u_2 = *++it;
            int u_3 = *++it;

            if (g.getStatus(u_1) == UNDOMINATED && g.getStatus(u_2) == UNDOMINATED &&
                g.getStatus(u_3) == UNDOMINATED) {
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