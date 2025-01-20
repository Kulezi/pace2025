#ifndef _DS_H
#define _DS_H
#include "instance.h"
#include "rrules.h"
#include "setops.h"
#include "td.h"
#define dbg(x) #x << " = " << x << " "

namespace DomSet {

// TODO: Branching po min/max stopniu.
// Branching: Nie wchodź jak już wykraczasz za najlepszy wynik.
// Bonus: jak jakąs heura umiesz to zrobić dużo wczesniej to zrob.
// Może regułki albera w branchingu monza na biezaco.
constexpr int UNSET = -1, INF = 1'000'000;

struct Exact {
    size_t branch_calls = 0;
    size_t n_splits = 0;

    std::vector<RRules::Rule> rules, rules_branch;
    Exact(std::vector<RRules::Rule> _rules, std::vector<RRules::Rule> _rules_branch)
        : rules(_rules), rules_branch(_rules_branch) {}

    std::vector<int> solve(Instance g, std::ostream &out) {
        RRules::reduce(g, rules);
        solve_tw(g, g.ds);
        print(g.ds, out);
        return g.ds;
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

            g.ds.insert(g.ds.end(), ds.begin() + g.ds.size(), ds.end());
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

    int pow3(int y) {
        int res = 1;
        while (y > 0) res *= 3, y--;
        return res;
    }

    std::vector<std::vector<int>> c;

    using TernaryFun = int;

    // e.g. WHITE = 0, GRAY = 0_dash, BLACK = 1 in platypus book.
    enum class Color { WHITE, GRAY, BLACK };

    // Remove x'th argument of f from the domain.
    TernaryFun cut(TernaryFun f, int x) {
        // Value of the first x-1 trits.
        int pref = f % pow3(x);

        // Value of trits x+1, x+2, ...
        int suf = (f / pow3(x + 1)) * pow3(x);

        return pref + suf;
    }

    // Insert at position x.
    TernaryFun insert(TernaryFun f, int x, Color c) {
        // Value of the first x-1 trits.
        int pref = f % pow3(x);
        // Every trit outside of prefix gets shifted right.
        int suf = (f - pref) * 3;

        // Lastly we add the value of inserted trit.
        return pref + suf + (int)c * pow3(x);
    }

    TernaryFun set(TernaryFun f, int x, Color c) { return f + ((int)c - (int)at(f, x)) * pow3(x); }

    Color at(TernaryFun f, int x) { return (Color)(f / pow3(x) % 3); }

    char val(Color c) {
        if ((int)c == 0) return '0';
        if ((int)c == 1) return '^';
        return '1';
    }

    TernaryFun toInt(std::string s) {
        int res = 0;
        for (int i = 0; i < s.size(); i++) {
            res += pow3(i) * (s[i] - '0');
        }
        return res;
    }

    std::string toString(TernaryFun f) {
        if (f == 0) return "0";
        std::string res;
        while (f > 0) {
            res += (char)((f % 3) + '0');
            f /= 3;
        }

        return res;
    }

    int calc_c(const Instance &g, TreeDecomposition &td, int t, TernaryFun f) {
        const auto &node = td[t];
        assert(f < pow3(node.bag.size()));
        if (!c[t].empty() && c[t][f] != UNSET) return c[t][f];
        if (c[t].empty()) c[t].resize(pow3(node.bag.size()), UNSET);
        c[t][f] = INF;
        auto bag_pos = [&](const std::vector<int> &bag, int v) -> int {
            int pos = 0;
            while (bag[pos] != v) ++pos;
            return pos;
        };

        switch (node.type) {
            case NodeType::Leaf:
                return c[t][f] = 0;  // OK
            case NodeType::IntroduceVertex: {
                int pos = bag_pos(node.bag, node.v);
                Color f_v = at(f, pos);
                // This vertex could already be dominated by some reduction rule.
                if (f_v == Color::WHITE && g.get_status(node.v) != DOMINATED)
                    return c[t][f] = INF;  // OK
                else {
                    return c[t][f] = calc_c(g, td, node.lChild, cut(f, pos));
                }
            }
            case NodeType::IntroduceEdge: {
                int pos_u = bag_pos(node.bag, node.to);
                int pos_v = bag_pos(node.bag, node.v);

                Color f_u = at(f, pos_u);
                Color f_v = at(f, pos_v);

                if (f_u == Color::BLACK && f_v == Color::WHITE)
                    return c[t][f] = calc_c(g, td, node.lChild, set(f, pos_v, Color::GRAY));
                else if (f_u == Color::WHITE && f_v == Color::BLACK)
                    return c[t][f] = calc_c(g, td, node.lChild, set(f, pos_u, Color::GRAY));
                else
                    return c[t][f] = calc_c(g, td, node.lChild, f);
            }
            case NodeType::Forget: {
                int pos_w = bag_pos(td[node.lChild].bag, node.v);
                return c[t][f] =
                           std::min(1 + calc_c(g, td, node.lChild, insert(f, pos_w, Color::BLACK)),
                                    calc_c(g, td, node.lChild, insert(f, pos_w, Color::WHITE)));
            }
            case NodeType::Join: {
                int zeros = 0;
                int N = node.bag.size();
                for (int i = 0; i < N; i++) {
                    if (at(f, i) == Color::WHITE) zeros++;
                }

                // Iterate over all combinations of choosing f_1(v), f_2(v) for positions where f(v)
                // = 0.
                for (int mask = 0; mask < (1 << zeros); mask++) {
                    int zero = 0;
                    // The value of f_1, f_2 will be the same on all trits that ain't 0 in f, so we
                    // don't need to touch those.
                    TernaryFun f_1 = f, f_2 = f;
                    for (int i = 0; i < N; i++) {
                        if (at(f, i) == Color::WHITE) {
                            if (mask >> zero & 1) {
                                f_1 = set(f_1, i, Color::GRAY);
                                f_2 = set(f_2, i, Color::WHITE);
                            } else {
                                f_1 = set(f_1, i, Color::WHITE);
                                f_2 = set(f_2, i, Color::GRAY);
                            }
                            ++zero;
                        }
                    }

                    c[t][f] = std::min(c[t][f],
                                       calc_c(g, td, node.lChild, f_1) + calc_c(g, td, node.rChild, f_2));
                }
                return c[t][f];
            }
        }

        assert(false);
        throw("Unknown node type reached in calc_c!");
    }

    void recover_ds(const Instance &g, TreeDecomposition &td, int t, TernaryFun f, std::vector<int> &ds) {
        auto &node = td[t];
        assert(f < pow3(node.bag.size()));
        assert(!c[t].empty() && c[t][f] != UNSET);
        auto bag_pos = [&](const std::vector<int> &bag, int v) -> int {
            int pos = 0;
            while (bag[pos] != v) ++pos;
            return pos;
        };

        switch (node.type) {
            case NodeType::IntroduceVertex: {
                int pos = bag_pos(node.bag, node.v);
                Color f_v = at(f, pos);
                recover_ds(g, td, node.lChild, cut(f, pos), ds);
                return;
            }
            case NodeType::IntroduceEdge: {
                int pos_u = bag_pos(node.bag, node.to);
                int pos_v = bag_pos(node.bag, node.v);

                Color f_u = at(f, pos_u);
                Color f_v = at(f, pos_v);

                if (f_u == Color::BLACK && f_v == Color::WHITE)
                    recover_ds(g, td, node.lChild, set(f, pos_v, Color::GRAY), ds);
                else if (f_u == Color::WHITE && f_v == Color::BLACK)
                    recover_ds(g, td, node.lChild, set(f, pos_u, Color::GRAY), ds);
                else
                    recover_ds(g, td, node.lChild, f, ds);
                return;
            }
            case NodeType::Forget: {
                int pos_w = bag_pos(td[node.lChild].bag, node.v);
                if (c[t][f] == 1 + calc_c(g, td, node.lChild, insert(f, pos_w, Color::BLACK))) {
                    ds.push_back(node.v);
                    recover_ds(g, td, node.lChild, insert(f, pos_w, Color::BLACK), ds);
                } else {
                    recover_ds(g, td, node.lChild, insert(f, pos_w, Color::WHITE), ds);
                }
                return;
            }
            case NodeType::Join: {
                int zeros = 0;
                int N = node.bag.size();
                for (int i = 0; i < N; i++) {
                    if (at(f, i) == Color::WHITE) zeros++;
                }

                // Iterate over all combinations of choosing f_1(v), f_2(v) for positions where f(v)
                // = 0.
                for (int mask = 0; mask < (1 << zeros); mask++) {
                    int zero = 0;
                    // The value of f_1, f_2 will be the same on all trits that ain't 0 in f, so we
                    // don't need to touch those.
                    TernaryFun f_1 = f, f_2 = f;
                    for (int i = 0; i < N; i++) {
                        if (at(f, i) == Color::WHITE) {
                            if (mask >> zero & 1) {
                                f_1 = set(f_1, i, Color::GRAY);
                                f_2 = set(f_2, i, Color::WHITE);
                            } else {
                                f_1 = set(f_1, i, Color::WHITE);
                                f_2 = set(f_2, i, Color::GRAY);
                            }
                            ++zero;
                        }
                    }

                    if (c[t][f] == calc_c(g, td, node.lChild, f_1) + calc_c(g, td, node.rChild, f_2)) {
                        recover_ds(g, td, node.lChild, f_1, ds);
                        recover_ds(g, td, node.rChild, f_2, ds);
                        return;
                    }
                }

                assert(false && "didn't find join argument!");
            }
            default:
                return;
        }
    }

    void solve_tw(Instance g, std::vector<int> &res) {
        // RRules::reduce(g, rules);
        TreeDecomposition td(g);
        c.resize(td.n_nodes());
        calc_c(g, td, td.root, 0);
        res = g.ds;
        recover_ds(g, td, td.root, 0, res);
    }
};
}  // namespace DomSet
#endif  // _DS_H