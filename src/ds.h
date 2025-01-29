#ifndef DS_H
#define DS_H
#include "instance.h"
#include "rrules.h"
#include "setops.h"
#include "td.h"
namespace DomSet {

constexpr int UNSET = -1, INF = 1'000'000;

#ifdef DS_BENCHMARK
struct BenchmarkInfo {
    size_t branch_calls = 0;
    size_t n_splits = 0;
    size_t max_encountered_treewidth = 0;
    using millis = std::chrono::duration<double, std::milli>;
    std::vector<millis> rule_time;
    std::vector<millis> rule_branch_time;
    millis treewidth_decomposition_time;
    millis treewidth_calculation_time;
    millis treewidth_joins_time;

    BenchmarkInfo() = default;
    BenchmarkInfo(std::vector<RRules::Rule> rules, std::vector<RRules::Rule> rules_branch)
        : rule_time(rules.size(), millis(0)),
          rule_branch_time(rules_branch.size(), millis(0)),
          treewidth_decomposition_time(0),
          treewidth_calculation_time(0),
          treewidth_joins_time(0) {}
};
#endif

struct Exact {
    std::vector<RRules::Rule> rules, rules_branch;
#ifdef DS_BENCHMARK
    BenchmarkInfo benchmark_info;
#endif  // DS_BENCHMARK
    Exact(std::vector<RRules::Rule> _rules, std::vector<RRules::Rule> _rules_branch)
        : rules(_rules), rules_branch(_rules_branch) {
#ifdef DS_BENCHMARK
        benchmark_info = BenchmarkInfo(rules, rules_branch);
#endif
    }

    std::vector<int> solve(Instance g, std::ostream &out) {
        auto split = g.split();
        if (split.size() <= 1) {
            reduce(g);
            solveTreewidth(g);
        } else {
            for (auto &cc : split) {
                g.nodes = cc;
                reduce(g);
                solveTreewidth(g);
            }
        }

        print(g.ds, out);
        return g.ds;
    }

    void reduce(Instance &g) {
    _start:
        for (size_t i = 0; i < rules.size(); i++) {
            auto &f = rules[i];
#ifdef DS_BENCHMARK
            auto start = std::chrono::high_resolution_clock::now();
            bool reduced = f(g);
            benchmark_info.rule_time[i] += std::chrono::high_resolution_clock::now() - start;
#else
            bool reduced = f(g);
#endif
            if (reduced) goto _start;
        }
    }

    void reduce_branch(Instance &g) {
    _start:
        for (size_t i = 0; i < rules_branch.size(); i++) {
            auto &f = rules_branch[i];
#ifdef DS_BENCHMARK
            auto start = std::chrono::high_resolution_clock::now();
            bool reduced = f(g);
            benchmark_info.rule_branch_time[i] += std::chrono::high_resolution_clock::now() - start;
#else
            bool reduced = f(g);
#endif
            if (reduced) goto _start;
        }
    }

    void take(Instance g, int v, std::vector<int> &best_ds, int level) {
        g.take(v);

        auto split = g.split();
#ifdef DS_BENCHMARK
        if (split.size() >= 2) {
            benchmark_info.n_splits++;
        }
#endif
        if (split.size() <= 1) {
            reduce_branch(g);
            solveBranching(g, best_ds, level);
            return;
        }

        // TODO: each components needs at least 1 node, hence if |ds| + |#ccs| > |best| return.
        for (auto &cc : split) {
            std::vector<int> ds;
            g.nodes = cc;
            reduce_branch(g);
            solveBranching(g, ds, level + 1);

            g.ds.insert(g.ds.end(),
                        ds.begin() + static_cast<std::vector<int>::difference_type>(g.ds.size()),
                        ds.end());
            if (!best_ds.empty() && g.ds.size() >= best_ds.size()) return;
        }

        best_ds = g.ds;
    }

    // TODO: Use heuristics to quit branching as soon as we know that the current branch is not
    // optimal.
    void solveBranching(const Instance g, std::vector<int> &best_ds, int level = 0) {
#if DS_BENCHMARK
        benchmark_info.branch_calls++;
#endif
        int v = g.minDegNodeOfStatus(UNDOMINATED);
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

    void solveBruteforce(Instance g, std::ostream &out) {
        int n = g.nodeCount();
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

    size_t pow3(size_t y) {
        size_t res = 1;
        while (y > 0) res *= 3, y--;
        return res;
    }

    std::vector<std::vector<int>> c;

    using TernaryFun = size_t;

    // e.g. WHITE = 0, GRAY = 0_dash, BLACK = 1 in platypus book.
    enum class Color { WHITE, GRAY, BLACK };

    // Remove x'th argument of f from the domain.
    TernaryFun cut(TernaryFun f, size_t x) {
        // Value of the first x-1 trits.
        TernaryFun pref = f % pow3(x);

        // Value of trits x+1, x+2, ...
        TernaryFun suf = (f / pow3(x + 1)) * pow3(x);

        return pref + suf;
    }

    // Insert at position x.
    TernaryFun insert(TernaryFun f, size_t x, Color c) {
        // Value of the first x-1 trits.
        TernaryFun pref = f % pow3(x);
        // Every trit outside of prefix gets shifted right.
        TernaryFun suf = (f - pref) * 3;

        // Lastly we add the value of inserted trit.
        return pref + suf + (size_t)c * pow3(x);
    }

    TernaryFun set(TernaryFun f, size_t x, Color c) {
        return f + ((int)c - (int)at(f, x)) * pow3(x);
    }

    Color at(TernaryFun f, size_t x) { return (Color)(f / pow3(x) % 3); }

    char val(Color c) {
        if ((int)c == 0) return '0';
        if ((int)c == 1) return '^';
        return '1';
    }

    TernaryFun toInt(std::string s) {
        TernaryFun res = 0;
        for (size_t i = 0; i < s.size(); i++) {
            res += pow3(i) * static_cast<size_t>(s[i] - '0');
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

    // [Parameterized Algorithms [7.3.2] - 10.1007/978-3-319-21275-3]
    int getC(const Instance &g, TreeDecomposition &td, int t, TernaryFun f) {
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
                return c[t][f] = 0;
            case NodeType::IntroduceVertex: {
                int pos = bag_pos(node.bag, node.v);
                Color f_v = at(f, pos);
                // This vertex could already be dominated by some reduction rule.
                if (f_v == Color::WHITE && g.getStatus(node.v) != DOMINATED)
                    return c[t][f] = INF;
                else {
                    return c[t][f] = getC(g, td, node.l_child, cut(f, pos));
                }
            }
            case NodeType::IntroduceEdge: {
                int pos_u = bag_pos(node.bag, node.to);
                int pos_v = bag_pos(node.bag, node.v);

                Color f_u = at(f, pos_u);
                Color f_v = at(f, pos_v);

                if (f_u == Color::BLACK && f_v == Color::WHITE)
                    return c[t][f] = getC(g, td, node.l_child, set(f, pos_v, Color::GRAY));
                else if (f_u == Color::WHITE && f_v == Color::BLACK)
                    return c[t][f] = getC(g, td, node.l_child, set(f, pos_u, Color::GRAY));
                else
                    return c[t][f] = getC(g, td, node.l_child, f);
            }
            case NodeType::Forget: {
                int pos_w = bag_pos(td[node.l_child].bag, node.v);
                return c[t][f] =
                           std::min(1 + getC(g, td, node.l_child, insert(f, pos_w, Color::BLACK)),
                                    getC(g, td, node.l_child, insert(f, pos_w, Color::WHITE)));
            }
            case NodeType::Join: {
#if DS_BENCHMARK
                auto start = std::chrono::high_resolution_clock::now();
#endif
                int zeros = 0;
                size_t N = node.bag.size();
                for (size_t i = 0; i < N; ++i) {
                    if (at(f, i) == Color::WHITE) zeros++;
                }
#if DS_BENCHMARK
                benchmark_info.treewidth_joins_time +=
                    std::chrono::high_resolution_clock::now() - start;
#endif
                // Iterate over all combinations of choosing f_1(v), f_2(v) for positions where
                // f(v) = 0.
                for (int mask = 0; mask < (1 << zeros); mask++) {
#if DS_BENCHMARK
                    start = std::chrono::high_resolution_clock::now();
#endif
                    int zero = 0;
                    // The value of f_1, f_2 will be the same on all trits that ain't 0 in f, so
                    // we don't need to touch those.
                    TernaryFun f_1 = f, f_2 = f;
                    for (size_t i = 0; i < N; ++i) {
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

#if DS_BENCHMARK
                    benchmark_info.treewidth_joins_time +=
                        std::chrono::high_resolution_clock::now() - start;
#endif
                    c[t][f] = std::min(
                        c[t][f], getC(g, td, node.l_child, f_1) + getC(g, td, node.r_child, f_2));
                }
                return c[t][f];
            }
        }

        assert(false);
        throw("Unknown node type reached in calc_c!");
    }

    void recoverDS(Instance &g, TreeDecomposition &td, int t, TernaryFun f) {
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
                recoverDS(g, td, node.l_child, cut(f, pos));
                return;
            }
            case NodeType::IntroduceEdge: {
                int pos_u = bag_pos(node.bag, node.to);
                int pos_v = bag_pos(node.bag, node.v);

                Color f_u = at(f, pos_u);
                Color f_v = at(f, pos_v);

                if (f_u == Color::BLACK && f_v == Color::WHITE)
                    recoverDS(g, td, node.l_child, set(f, pos_v, Color::GRAY));
                else if (f_u == Color::WHITE && f_v == Color::BLACK)
                    recoverDS(g, td, node.l_child, set(f, pos_u, Color::GRAY));
                else
                    recoverDS(g, td, node.l_child, f);
                return;
            }
            case NodeType::Forget: {
                int pos_w = bag_pos(td[node.l_child].bag, node.v);
                if (c[t][f] == 1 + getC(g, td, node.l_child, insert(f, pos_w, Color::BLACK))) {
                    g.ds.push_back(node.v);
                    recoverDS(g, td, node.l_child, insert(f, pos_w, Color::BLACK));
                } else {
                    recoverDS(g, td, node.l_child, insert(f, pos_w, Color::WHITE));
                }
                return;
            }
            case NodeType::Join: {
                int zeros = 0;
                size_t N = node.bag.size();
                for (size_t i = 0; i < N; ++i) {
                    if (at(f, i) == Color::WHITE) zeros++;
                }

                // Iterate over all combinations of choosing f_1(v), f_2(v) for positions where
                // f(v) = 0.
                for (int mask = 0; mask < (1 << zeros); mask++) {
                    int zero = 0;
                    // The value of f_1, f_2 will be the same on all trits that ain't 0 in f, so
                    // we don't need to touch those.
                    TernaryFun f_1 = f, f_2 = f;
                    for (size_t i = 0; i < N; ++i) {
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

                    if (c[t][f] ==
                        getC(g, td, node.l_child, f_1) + getC(g, td, node.r_child, f_2)) {
                        recoverDS(g, td, node.l_child, f_1);
                        recoverDS(g, td, node.r_child, f_2);
                        return;
                    }
                }

                assert(false && "didn't find join argument!");
            }
            default:
                return;
        }
    }

    void solveTreewidth(Instance &g) {
#ifdef DS_BENCHMARK
        auto start = std::chrono::high_resolution_clock::now();
        TreeDecomposition td(g);
        benchmark_info.treewidth_decomposition_time +=
            std::chrono::high_resolution_clock::now() - start;
        start = std::chrono::high_resolution_clock::now();
        c = std::vector<std::vector<int>>(td.n_nodes(), std::vector<int>());

        getC(g, td, td.root, 0);
        recoverDS(g, td, td.root, 0);
        benchmark_info.treewidth_calculation_time +=
            std::chrono::high_resolution_clock::now() - start;
        benchmark_info.max_encountered_treewidth =
            std::max(benchmark_info.max_encountered_treewidth, td.width());
#else
        TreeDecomposition td(g);
        c = std::vector<std::vector<int>>(td.n_nodes(), std::vector<int>());

        getC(g, td, td.root, 0);
        recoverDS(g, td, td.root, 0);
#endif
    }
};
}  // namespace DomSet
#endif  // DS_H