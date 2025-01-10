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
    size_t branch_calls = 0;
    size_t n_splits = 0;

    std::vector<RRules::Rule> rules, rules_branch;
    Exact(std::vector<RRules::Rule> _rules, std::vector<RRules::Rule> _rules_branch)
        : rules(_rules), rules_branch(_rules_branch) {}

    std::vector<int> solve(Instance g, std::ostream &out) {
        std::vector<int> ds;
        RRules::reduce(g, rules);

        solve_branching(g, ds);
        print(ds, out);
        return ds;
    }

    void take(Instance g, int v, std::vector<int> &best_ds, int level) {
        g.take(v);

        auto split = g.split();
#if BENCH
        if (split.size() >= 2) {
            n_splits++;
        }
#endif

        // TODO: each components needs at least 1 node, hence if |ds| + |#ccs| > |best| return.
        for (auto &cc : split) {
            std::vector<int> ds;
            g.nodes = cc;
            RRules::reduce(g, rules_branch);
            solve_branching(g, ds, level + 1);

            g.ds.insert(g.ds.end(), ds.begin()+g.ds.size(), ds.end());
            if (!best_ds.empty() && g.ds.size() >= best_ds.size()) return;
        }

        best_ds = g.ds;
    }

    void solve_branching(const Instance g, std::vector<int> &best_ds, int level = 0) {
#if BENCH
        branch_calls++;
#endif
        int v = g.min_deg_node_of_status(UNDOMINATED);
        if (!best_ds.empty() && g.ds.size() >= best_ds.size()) return;
        if (v == -1) {
            best_ds = g.ds;
            return;
        }

        // Branch 0: v belongs to DS -> dominate N(v)
        if (g.deg(v) != 1) take(g, v, best_ds, level + 1);

        // Branches 1, ..., deg(v): v doesn't belong to DS -> take any neighbour to DS.
        for (auto u : g.adj[v]) {
            take(g, u, best_ds, level + 1);
        }
    }

    void print(std::vector<int> ds, std::ostream &out) {
        out << ds.size() << "\n";
        sort(ds.begin(), ds.end());
        for (auto i : ds) {
            out << i << "\n";
        }
    }

    void solve_bruteforce(Instance g, std::ostream &out) {
        int n = g.n_nodes();
        std::vector<int> best_ds;

        for (int mask = 0; mask < (1 << n); mask++) {
            std::vector<int> dominated(g.next_free_id, false);
            auto node = g.nodes.begin();

            std::vector<int> ds;
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

        print(best_ds, out);
    }
};
}  // namespace DomSet
#endif  // _DS_H