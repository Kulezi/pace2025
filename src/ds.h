#ifndef _DS_H
#define _DS_H
#include "graph.h"
#include "rrules.h"
#include "setops.h"
#define dbg(x) #x << " = " << x << " "

namespace DomSet {

// TODO: Branching po min/max stopniu.
// Branching: Nie wchodź jak już wykraczasz za najlepszy wynik.
// Bonus: jak jakąs heura umiesz to zrobić dużo wczesniej to zrob.
// Może regułki albera w branchingu monza na biezaco.

struct Exact {
    std::vector<RRules::Rule> rules;
    Exact(vector<RRules::Rule> _rules) : rules(_rules) {}

    void solve(Graph g, std::ostream &out) {
        solve_branching(g, {});
        print(out);
    }

    std::vector<int> best_ds;
    void take(Graph &g, vector<int> &ds, int v) {
        auto n_g = g;
        auto n_ds = ds;
        n_ds.push_back(v);

        for (auto w : n_g.neighbourhood_excluding(v)) n_g.set_color(w, DOMINATED);
        n_g.remove_node(v);

        RRules::reduce(n_g, n_ds, rules);
        solve_branching(n_g, n_ds);
    }

    void solve_branching(Graph g, vector<int> cur_ds) {
        int v = g.min_deg_node_of_color(UNDOMINATED);
        if (cur_ds.size() >= best_ds.size() && !best_ds.empty()) return;
        if (v == -1) {
            // Hooray, we have a dominating set.
            best_ds = cur_ds;
            std::cerr << " -> " << best_ds.size();
            return;
        }

        // Branch 0: v belongs to DS -> dominate N(v)
        take(g, cur_ds, v);

        // Branches 1, ..., deg(v): v doesn't belong to DS -> take any neighbour to DS.

        vector<int> N = {std::begin(g.adj[v]), std::end(g.adj[v])};
        std::sort(N.begin(), N.end(), [&](int lhs, int rhs) { return g.deg(lhs) > g.deg(rhs); });
        for (auto u : N) {
            take(g, cur_ds, u);
        }
        // TODO: maybe take neighbours in order of decreasing degree?
    }

    void print(std::ostream &out) {
        out << best_ds.size() << "\n";
        sort(best_ds.begin(), best_ds.end());
        for (auto i : best_ds) {
            out << i << "\n";
        }
    }

    void solve_bruteforce(Graph g, std::ostream &out) {
        int n = g.n_nodes;

        assert(g.n_nodes == (int)g.nodes.size());
        for (int mask = 0; mask < (1 << n); mask++) {
            vector<int> dominated(g.next_free_id, false);
            auto node = g.nodes.begin();

            vector<int> ds;
            for (int i = 0; i < n; i++) {
                int v = *node;
                if (mask >> i & 1) {
                    ds.push_back(v);
                    dominated[v] = true;
                    for (auto u : g.adj[v]) dominated[u] = true;
                }

                ++node;
            }

            bool is_domset = true;
            for (auto v : g.nodes) {
                if (!dominated[v]) {
                    is_domset = false;
                    break;
                }
            }

            if (is_domset && (best_ds.empty() || ds.size() < best_ds.size())) {
                best_ds = ds;
            }
        }

        print(out);
    }
};
}  // namespace DomSet
#endif  // DS_H