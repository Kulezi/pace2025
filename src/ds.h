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
    Graph g;
    std::vector<RRules::Rule> rules;
    std::vector<int> cur_ds;
    Exact(Graph _g, vector<RRules::Rule> _rules) : g(_g), rules(_rules) {
    // Graph preprocessing.

    std::cerr << "n = " << g.n_nodes << ", " << "m = " << g.n_edges <<" -> ";
    _start:
        for (auto f : rules) {
            if (f(g, cur_ds)) goto _start;
        }

    std::cerr << "n = " << g.n_nodes << ", " << "m = " << g.n_edges <<std::endl;

    }

    void solve(std::ostream &out) {
        solve_branching();
        print(out);
    }

    std::vector<int> best_ds;
    void take(int v) {
        int old_color = g.get_color(v);
        g.set_color(v, TAKEN);
        cur_ds.push_back(v);
        vector<int> untake;
        for (auto u : g.adj[v]) {
            if (g.get_color(u) == UNDOMINATED) {
                g.set_color(u, DOMINATED);
                untake.push_back(u);
            }
        }

        solve_branching();
        for (auto u : untake) g.set_color(u, UNDOMINATED);
        cur_ds.pop_back();
        g.set_color(v, old_color);
    }

    void solve_branching() {
        int v = min_deg_undominated_node();
        if (cur_ds.size() >= best_ds.size() && !best_ds.empty()) return;
        if (v == -1) {
            // Hura, mamy dominating set.
            best_ds = cur_ds;
            return;
        }

        // Branch 0: v należy do DS -> zdominuj n(v)
        take(v);

        // Branche 1, ..., deg(v): v nie należy do DS -> weź jakiegokolwiek sąsiada do DS.
        for (auto u : g.adj[v]) {
            take(u);
        }
        // (najlepsza heura to chyba tego o max liczbie niezdominowanych sasiadow).
    }

    int min_deg_undominated_node() {
        int best_v = -1;
        for (auto v : g.nodes)
            if (g.get_color(v) == UNDOMINATED && (best_v == -1 || g.deg(v) < g.deg(best_v))) best_v = v;

        return best_v;
    }

    void print(std::ostream &out) {
        out << best_ds.size() << "\n";
        sort(best_ds.begin(), best_ds.end());
        for (auto i : best_ds) {
            out << i << "\n";
        }
    }
    // void solve_bruteforce(Graph &g, std::ostream &out) {
    //     int n = g.n_nodes;

    //     vector<int> best_ds;
    //     assert(g.n_nodes == (int)g.nodes.size());
    //     for (int mask = 0; mask < (1 << n); mask++) {
    //         vector<int> dominated(g.next_free_id, false);
    //         auto node = g.nodes.begin();

    //         vector<int> ds;
    //         for (int i = 0; i < n; i++) {
    //             int v = *node;
    //             if (mask >> i & 1) {
    //                 ds.push_back(v);
    //                 dominated[v] = true;
    //                 for (auto u : g.adj[v]) dominated[u] = true;
    //             }

    //             ++node;
    //         }

    //         std::cerr << mask << "\n";
    //         bool is_domset = true;
    //         for (auto v : g.nodes) {
    //             if (!dominated[v]) {
    //                 is_domset = false;
    //                 break;
    //             }
    //         }

    //         if (is_domset && (best_ds.empty() || ds.size() < best_ds.size())) {
    //             best_ds = ds;
    //         }
    //     }

    //     out << best_ds.size() << "\n";
    //     for (auto i : best_ds) out << i << "\n";
    // }

    // void solve_branching(Graph &g, std::ostream &out) {
    //     int v = min_deg_node(g);

    //     // Branch 0: v należy do DS -> zdominuj n(v)
    //     // Branche 1, ..., deg(v): v nie należy do DS -> weź jakiegokolwiek sąsiada do DS.
    //     // (najlepsza heura to chyba tego o max liczbie niezdominowanych sasiadow).
    //     vector<int> taken;
    //     for (auto u : g.adj[v]) {
    //     }
    // }

    // void solve(Graph &g, std::ostream &out) { solve_bruteforce(g, out); }
};
}  // namespace DomSet
#endif  // DS_H