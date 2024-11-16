#ifndef _RRULES_H
#define _RRULES_H
#include "graph.h"
#include "setops.h"

enum { UNDOMINATED, DOMINATED, TAKEN };
namespace RRules {
using Rule = std::function<bool(Graph &, std::vector<int> &)>;

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
            // We add one vertex so removing only one can force us into an infinite loop.
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

// // Naive implementation of Main Rule 2 - DOI 10.1007/s10479-006-0045-4, p. 4
// // ~ O(|V|^2) or O(|V|^3) depending on the remove_node operation complexity.
// bool AlberMainRule2(Graph &g, std::vector<int> &ds) {
//     for (auto v : g.nodes) {
//         for (auto w : g.nodes) {
//             if (v == w) continue;
//             auto N_vw_with = unite(g.neighbourhood_including(v), g.neighbourhood_including(w));
//             auto N_vw_without = unite(g.neighbourhood_excluding(v),
//             g.neighbourhood_excluding(w));

//             std::list<int> N_exit, N_guard, N_prison;
//             for (auto u : N_vw_without) {
//                 auto N_u = g.neighbourhood_excluding(u);

//                 if (!remove(N_u, N_vw_with).empty()) N_exit.push_back(u);
//             }

//             for (auto u : remove(N_vw_without, N_exit)) {
//                 auto N_u = g.neighbourhood_excluding(u);
//                 if (!intersect(N_u, N_exit).empty()) N_guard.push_back(u);
//             }

//             N_prison = remove(remove(N_vw_without, N_exit), N_guard);
// 00
//             if (!N_prison.empty() && has_undominated_node(g, N_prison)) {
//                 // We add one vertex so removing only one can force us into an infinite loop.
//                 for (auto u : N_prison) g.remove_node(u);
//                 for (auto u : N_guard) g.remove_node(u);

//                 for (auto u : g.adj[v]) g.set_color(u, DOMINATED);
//                 ds.push_back(v);

//                 g.remove_node(v);
//                 return true;
//             }
//         }
//     }

//     return false;
// }

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

// Naive implementation of Simple Rule 1 - DOI 10.1007/s10479-006-0045-4, p. 6
// ~ O(|V| * (# removed nodes)) depending on the remove_node operation complexity.
bool AlberSimpleRule2(Graph &g, std::vector<int> &) {
    std::vector<int> to_remove;
    for (auto v : g.nodes) {
        if (g.get_color(v) == DOMINATED) {
            to_remove.push_back(v);
        }
    }

    for (auto v : to_remove) g.remove_node(v);
    return !to_remove.empty();
}

}  // namespace RRules

#endif