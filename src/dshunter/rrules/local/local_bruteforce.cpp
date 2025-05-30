#include <climits>
#include <iostream>
#include <queue>

#include "../../utils.h"
#include "../rrules.h"
#include "dshunter/solver/treewidth/td/flow_cutter_decomposer.h"
namespace {
using namespace DSHunter;
std::tuple<std::vector<int>, std::vector<int>, std::vector<int>> PEG(const Instance &g, const std::vector<int> &V) {
    std::vector<int> P, E, G, N_closed;
    for (auto v : V) {
        N_closed = unite(N_closed, g[v].n_closed);
    }

    auto N_open = remove(N_closed, V);
    auto isExit = [&](int u) {
        for (auto v : g[u].n_open) {
            if (!std::ranges::binary_search(N_open, v))
                return true;
        }
        return false;
    };

    for (auto u : N_open) {
        if (isExit(u))
            E.push_back(u);
    }

    for (auto u : remove(N_open, E)) {
        if (remove(g[u].n_open, E).empty()) {
            P.push_back(u);
        } else {
            G.push_back(u);
        }
    }

    return { P, E, G };
}

std::vector<std::vector<int>> getBags(const Instance &input_graph) {
    auto cfg = SolverConfig();
    cfg.decomposition_time_budget = std::chrono::seconds(1);
    FlowCutterDecomposer xd(&cfg);
    auto td = xd.decompose(input_graph);
    if (!td.has_value())
        return {};

    auto bags = td->bag;
    for (auto &bag : bags) {
        std::ranges::sort(bag);
    }

    return bags;
}

void maximize(int &x, const int y) {
    if (y >= 0) {
        x = std::max(x, y);
    }
}

void minimize(int &x, const int y) {
    if (y >= 0) {
        x = std::min(x, y);
    }
}

// Returns statuses {dominated, taken} for all nodes from N assuming "hardest assignment" of the separator, i.e., not taking anything from it.
std::pair<std::vector<bool>, std::vector<bool>> hardCase(const Instance &g, const std::vector<int> &N) {
    std::vector d(g.all_nodes.size(), false), t(g.all_nodes.size(), false);
    for (auto v : N) d[v] = g.isDominated(v);
    return { d, t };
}

// Returns statuses {dominated, taken} for all nodes from N assuming "easiest assignment" of the separator, i.e., taking everything from it,
// or dominating it from the outside of N if taking is impossible.
std::pair<std::vector<bool>, std::vector<bool>> easyCase(const DSHunter::Instance &g, const std::vector<int> &N, const std::vector<int> &E) {
    auto [d, t] = hardCase(g, N);
    for (auto u : E) {
        if (!g.isDisregarded(u)) {
            t[u] = true;
            for (auto v : g[u].n_closed) d[v] = true;
        } else {
            bool can_be_dominated = false;
            for (auto v : DSHunter::remove(g[u].n_open, N)) {
                if (!g.isDisregarded(v)) {
                    can_be_dominated = true;
                    break;
                }
            }
            if (can_be_dominated)
                d[u] = true;
        }
    }

    return { d, t };
}

// Checks whether taking assignment m for set A doesn't break constraints already posed by g.
bool isCompatible(const Instance &g, const std::vector<int> &A, const int m) {
    for (auto i = 0; i < A.size(); ++i) {
        const int u = A[i];
        if (m >> i & 1 && g.isDisregarded(u))
            return false;
        if (!(m >> i & 1)) {
            for (auto [v, s] : g[u].adj) {
                if (s == EdgeStatus::FORCED && g.isDisregarded(v))
                    return false;
            }
        }
    }
    return true;
}

// Returns the number of nodes taken in A if N is dominated by m_take, -1 otherwise.
int solveMask(const Instance &g, const std::vector<int> &A, const std::vector<int> &N, const int m_take, std::vector<bool> dominated, std::vector<bool> taken) {
    if (!isCompatible(g, A, m_take))
        return -1;
    int ds_size = 0;
    for (auto i = 0; i < A.size(); ++i) {
        const int u = A[i];
        if (m_take >> i & 1) {
            ++ds_size;
            taken[u] = true;
            if (g.isDisregarded(u))
                return -1;
            for (auto v : g[u].n_closed) {
                dominated[v] = true;
            }
        }
    }

    for (auto u : N) {
        if (!dominated[u])
            return -1;
        for (auto [v, s] : g[u].adj) {
            if (s == EdgeStatus::FORCED && contains(N, { v }) && !taken[u] && !taken[v])
                return -1;
        }
    }

    return ds_size;
}

bool trim(Instance &g, const std::vector<int> &A, const int x, const int y) {
    bool did_something = false;
    for (int i = 0; i < A.size(); i++) {
        if (y >> i & 1) {
            int u = A[i];
            if ((x >> i & 1) && g.hasNode(u)) {
                did_something = true;
                g.take(A[i]);
            } else if (g.hasNode(u) && !g.isDisregarded(u)) {
                g.markDisregarded(u);
                did_something = true;
                for (auto [v, s] : g[u].adj) {
                    if (s == EdgeStatus::FORCED) {
                        DS_ASSERT(!g.isDisregarded(v));
                        g.take(v);
                    }
                }
            }
        }
    }

    return did_something;
}

};  // namespace

namespace DSHunter {
int R = 0;
bool trimSubset(Instance &g, const std::vector<int> &V) {
    if (V.empty() || V.size() > 10)
        return false;
    for (auto v : V)
        if (!g.hasNode(v))
            return false;

    auto [P, E, G] = PEG(g, V);
    auto A_temp = unite(P, unite(G, V));
    auto N = unite(A_temp, E);
    std::vector<int> A;
    for (auto a : A_temp)
        if (!g.isDisregarded(a))
            A.push_back(a);

    if (A.size() > 10)
        return false;

    auto [d_hard, t_hard] = hardCase(g, N);
    auto [d_easy, t_easy] = easyCase(g, N, E);

    int sz = 1 << A.size();
    // {hard, easy}
    std::vector<std::pair<int, int>> results(sz);
    for (auto i = 0; i < sz; ++i) {
        results[i] = { solveMask(g, A, N, i, d_hard, t_hard),
                       solveMask(g, A, N, i, d_easy, t_easy) };
    }

    auto apply = [&](const int x, const int y) {
        int mi_easy = INT_MAX, mx_easy = INT_MIN;
        int mi_hard = INT_MAX, mx_hard = INT_MIN;
        for (int m = 0; m < sz; m++) {
            if ((m & y) == x) {
                maximize(mx_hard, results[m].first);
                maximize(mx_easy, results[m].second);
            } else {
                minimize(mi_hard, results[m].first);
                minimize(mi_easy, results[m].second);
            }
        }

        if (mx_easy <= mi_easy && mx_hard <= mi_hard && mx_hard >= 0 && y > 0 && trim(g, A, x, y)) {
            return true;
        }

        return false;
    };

    for (int y = 1; y < sz; y++) {
        if (apply(0, y))
            return true;
        if (apply(y, y))
            return true;
    }

    return false;
}

std::vector<int> expand(const Instance &g, const std::vector<int> &V) {
    auto res = V;
    for (auto v : V) res = unite(res, g[v].n_closed);
    return res;
}

bool localBruteforceRule(Instance &g) {
    bool reduced = false;

    for (const auto bags = getBags(g); auto &bag : bags) {
        int bs = static_cast<int>(bag.size());
        if (bs <= 10) {
            for (int mask = 0; mask < (1 << bs); mask++) {
                std::vector<int> V;
                for (int j = 0; j < bs; j++) {
                    if (mask >> j & 1)
                        V.push_back(bag[j]);
                }

                if (trimSubset(g, V)) {
                    reduced = true;
                }
            }
        }
    }

    auto nodes = g.nodes;
    for (auto u : nodes) {
        if (g.hasNode(u)) {
            std::vector<int> one = { u };
            auto two = expand(g, one);
            auto three = expand(g, two);
            auto four = expand(g, three);
            if (trimSubset(g, four)) {
                reduced = true;
            }
        }
    }
    return reduced;
}

ReductionRule LocalBruteforceRule("localBruteforceRule", localBruteforceRule, 1, 1);

}  // namespace DSHunter
