#ifndef _RRULES_H
#define _RRULES_H
#include "graph.h"
#include "setops.h"
#define dbg(x) #x << " = " << x << " "
enum { UNDOMINATED, DOMINATED, TAKEN };
namespace RRules {
using Rule = std::function<bool(Graph &, std::vector<int> &)>;

void reduce(Graph &g, std::vector<int> &ds, std::vector<Rule> rules) {
_start:
    for (size_t i = 0; i < rules.size(); i++) {
        auto f = rules[i];
        bool reduced = f(g, ds);
        if (reduced) {
            std::cerr << "Applied rule " << i << " [";
            for (auto j : ds) std::cerr << j << " ";
            std::cerr << "]\n";
            goto _start;
        }
    }
}

bool has_undominated_node(Graph &g, std::list<int> nodes) {
    for (auto v : nodes)
        if (g.get_color(v) == UNDOMINATED) return true;
    return false;
}

// Naive implementation of Main Rule 1 - DOI 10.1007/s10479-006-0045-4, p. 4
// ~ O(|V|^2) or O(|V|^3) depending on the remove_node operation complexity.
bool AlberMainRule1(Graph &g, std::vector<int> &ds) {
    for (auto v : g.nodes) {
        auto N_v_with = g.neighbourhood_including(v);
        auto N_v_without = g.neighbourhood_excluding(v);

        std::list<int> N_exit, N_guard, N_prison;
        for (auto u : N_v_without) {
            auto N_u = g.neighbourhood_excluding(u);

            if (!remove(N_u, N_v_with).empty()) N_exit.push_back(u);
        }

        for (auto u : remove(N_v_without, N_exit)) {
            auto N_u = g.neighbourhood_excluding(u);
            if (!intersect(N_u, N_exit).empty()) N_guard.push_back(u);
        }

        N_prison = remove(remove(N_v_without, N_exit), N_guard);

        if (!N_prison.empty() && has_undominated_node(g, N_prison)) {
            for (auto u : N_prison) g.remove_node(u);
            for (auto u : N_guard) g.remove_node(u);

            for (auto u : g.adj[v]) g.set_color(u, DOMINATED);
            ds.push_back(v);

            g.remove_node(v);
            return true;
        }
    }

    return false;
}

// Naive implementation of Main Rule 2 - DOI 10.1007/s10479-006-0045-4, p. 4
// ~ O(|V|^2) or O(|V|^3) depending on the remove_node operation complexity.
bool AlberMainRule2(Graph &g, std::vector<int> &ds) {
    for (auto v : g.nodes) {
        for (auto w : g.nodes) {
            if (v == w) continue;
            auto N_v_without = g.neighbourhood_excluding(v);
            auto N_w_without = g.neighbourhood_excluding(w);

            auto N_vw_with = unite(g.neighbourhood_including(v), g.neighbourhood_including(w));
            auto N_vw_without = unite(N_v_without, N_w_without);

            std::list<int> N_exit, N_guard, N_prison;
            for (auto u : N_vw_without) {
                auto N_u = g.neighbourhood_excluding(u);

                if (!remove(N_u, N_vw_with).empty()) N_exit.push_back(u);
            }

            for (auto u : remove(N_vw_without, N_exit)) {
                auto N_u = g.neighbourhood_excluding(u);
                if (!intersect(N_u, N_exit).empty()) N_guard.push_back(u);
            }

            N_prison = remove(remove(N_vw_without, N_exit), N_guard);
            auto N_prison_intersect_B = N_prison;
            N_prison_intersect_B.remove_if([&](int u) { return g.get_color(u) != UNDOMINATED; });

            auto intersection_can_be_dominated_by_single_from = [&](std::list<int> &nodes) {
                for (auto x : nodes)
                    if (contains(g.neighbourhood_including(x), N_prison_intersect_B)) return true;
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
                    int z1 = g.add_node();
                    int z2 = g.add_node();
                    int z3 = g.add_node();

                    g.add_edge(v, z1);
                    g.add_edge(v, z2);
                    g.add_edge(v, z3);

                    g.add_edge(w, z1);
                    g.add_edge(w, z2);
                    g.add_edge(w, z3);

                    g.remove_nodes(N_prison);
                    g.remove_nodes(intersect(intersect(N_guard, N_v_without), N_w_without));
                } else if (can_be_dominated_by_just_v) {
                    // Case 1.2
                    ds.push_back(v);
                    for (auto u : N_v_without) g.set_color(u, DOMINATED);

                    // TODO: Order of those operations can be changed to reduce reduntant ops.
                    g.remove_node(v);
                    g.remove_nodes(N_prison);
                    g.remove_nodes(intersect(N_v_without, N_guard));
                } else if (can_be_dominated_by_just_w) {
                    // Case 1.3
                    ds.push_back(w);
                    for (auto u : N_w_without) g.set_color(u, DOMINATED);

                    // TODO: Order of those operations can be changed to reduce reduntant ops.
                    g.remove_node(w);
                    g.remove_nodes(N_prison);
                    g.remove_nodes(intersect(N_w_without, N_guard));
                } else {
                    // Case 2:
                    ds.push_back(v);
                    ds.push_back(w);

                    for (auto u : N_vw_without) g.set_color(u, DOMINATED);
                    g.set_color(v, TAKEN);
                    g.set_color(w, TAKEN);
                    g.remove_nodes(N_prison);
                    g.remove_nodes(N_guard);
                }

                return true;
            }
        }
    }

    return false;
}

// Naive implementation of Simple Rule 1 - DOI 10.1007/s10479-006-0045-4, p. 6
// ~ O(|E| + |V| * (# removed edges)) depending on the remove_node operation complexity.
bool AlberSimpleRule1(Graph &g, std::vector<int> &) {
    std::vector<std::pair<int, int>> to_remove;
    for (auto v : g.nodes) {
        for (auto w : g.adj[v]) {
            if (v > w) continue;
            if (g.get_color(v) == DOMINATED && g.get_color(w) == DOMINATED) {
                to_remove.emplace_back(v, w);
            }
        }
    }
    for (auto [v, w] : to_remove) g.remove_edge(v, w);

    return !to_remove.empty();
}

// Naive implementation of Simple Rule 2 - DOI 10.1007/s10479-006-0045-4, p. 6
// ~ O(|V| * (# removed nodes)) depending on the remove_node operation complexity.
bool AlberSimpleRule2(Graph &g, std::vector<int> &) {
    std::vector<int> to_remove;
    for (auto v : g.nodes) {
        if (g.get_color(v) == DOMINATED && g.deg(v) <= 1) {
            to_remove.push_back(v);
        }
    }

    for (auto v : to_remove) g.remove_node(v);
    return !to_remove.empty();
}

// Naive implementation of Simple Rule 3 - DOI 10.1007/s10479-006-0045-4, p. 6
// ~ O(|V|^2 * (# removed nodes)) depending on the remove_node operation complexity.
bool AlberSimpleRule3(Graph &g, std::vector<int> &) {
    std::vector<int> to_remove;
    for (auto v : g.nodes) {
        if (g.get_color(v) != UNDOMINATED && g.deg(v) == 2) {
            int u_1 = g.adj[v].front();
            int u_2 = *++g.adj[v].begin();
            if (g.get_color(u_1) == UNDOMINATED && g.get_color(u_2) == UNDOMINATED) {
                if (g.has_edge(u_1, u_2)) {
                    // 3.1
                    g.remove_node(v);
                    return true;
                } else {
                    // 3.2
                    for (auto u : g.adj[u_1]) {
                        if (g.has_edge(u, u_2)) {
                            g.remove_node(v);
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
bool AlberSimpleRule4(Graph &g, std::vector<int> &) {
    std::vector<int> to_remove;
    for (auto v : g.nodes) {
        if (g.get_color(v) != UNDOMINATED && g.deg(v) == 3) {
            auto it = g.adj[v].begin();
            int u_1 = *it;
            int u_2 = *++it;
            int u_3 = *++it;

            if (g.get_color(u_1) == UNDOMINATED && g.get_color(u_2) == UNDOMINATED &&
                g.get_color(u_3) == UNDOMINATED) {
                int n_edges = (g.has_edge(u_1, u_2) + g.has_edge(u_2, u_3) + g.has_edge(u_1, u_3));
                if (n_edges >= 2) {
                    to_remove.push_back(v);
                }
            }
        }
    }

    for (auto v : to_remove) g.remove_node(v);
    return !to_remove.empty();
}

}  // namespace RRules

#endif