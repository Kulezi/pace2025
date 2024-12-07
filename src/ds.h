#ifndef _DS_H
#define _DS_H
#include "instance.h"
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

    void solve(Instance g, std::ostream &out) {
        vector<int> ds;
        RRules::reduce(g, rules);
        solve_branching(g);
        print(out);
    }

    std::vector<int> best_ds;
    void take(Instance g, int v) {
        g.take(v);

        RRules::reduce(g, rules);
        solve_branching(g);
    }

    void solve_branching(Instance g) {
        int v = g.min_deg_node_of_color(UNDOMINATED);
        if (g.ds.size() >= best_ds.size() && !best_ds.empty()) return;
        if (v == -1) {
            // Hooray, we have a dominating set.
            best_ds = g.ds;
            std::cerr << "-> <" << best_ds.size() << "> ";
            return;
        }

        // Branch 0: v belongs to DS -> dominate N(v)
        if (g.deg(v) != 1) take(g, v);

        // Branches 1, ..., deg(v): v doesn't belong to DS -> take any neighbour to DS.
        for (auto u : g.adj[v]) {
            take(g, u);
        }
    }

    void print(std::ostream &out) {
        out << best_ds.size() << "\n";
        sort(best_ds.begin(), best_ds.end());
        for (auto i : best_ds) {
            out << i << "\n";
        }
    }

    void solve_bruteforce(Instance g, std::ostream &out) {
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