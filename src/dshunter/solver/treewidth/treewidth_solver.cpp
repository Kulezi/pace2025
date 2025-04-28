#include "treewidth_solver.h"
#include "td/exec_decomposer.h"
#include "td/flow_cutter_decomposer.h"
#include <memory>
#include "../../utils.h"

namespace {
std::unique_ptr<DSHunter::Decomposer> getDecomposer(const DSHunter::SolverConfig *cfg) {
    if (cfg->decomposer_path.empty()) return std::make_unique<DSHunter::FlowCutterDecomposer>(cfg);
    return std::make_unique<DSHunter::ExecDecomposer>(cfg);
}
}  // namespace

namespace DSHunter {
TreewidthSolver::TreewidthSolver(const SolverConfig *cfg) : cfg(cfg), decomposer(getDecomposer(cfg)) {}

constexpr int UNSET = -1, INF = 1'000'000'000;

// Returns true if instance was solved. Solution set is stored in given instance.
// Returns false if it would lead to exceeding the memory limit.
bool TreewidthSolver::solve(Instance &instance) {
    g = instance;

    auto o = decomposer->decompose(g);
    if (!o.has_value()){
        std::cerr << "Decomposition failed" << std::endl;
        return false;
}
    td = NiceTreeDecomposition::nicify(g, o.value());
    if (getMemoryUsage(td) > cfg->max_memory_in_bytes)
        return false;

    c = std::vector<std::vector<int>>(td.n_nodes(), std::vector<int>());

    getC(td.root, 0);
    recoverDS(td.root, 0);
    instance.ds = g.ds;
    return true;
}

inline int TreewidthSolver::cost(const Instance &g, int v) { return g[v].is_extra ? INF : 1; }

// [Parameterized Algorithms [7.3.2] - 10.1007/978-3-319-21275-3] extended to handle forced
// edges.
int TreewidthSolver::getC(int t, TernaryFun f) {
    const auto &node = td[t];
    DS_ASSERT(f < pow3[node.bag.size()]);

    if (!c[t].empty() && c[t][f] != UNSET)
        return c[t][f];
    if (c[t].empty())
        c[t].resize(pow3[node.bag.size()], UNSET);
    c[t][f] = INF;
    auto bag_pos = [&](const std::vector<int> &bag, int v) -> int {
        int pos = 0;
        while (bag[pos] != v) ++pos;
        return pos;
    };

    switch (node.type) {
        case NiceTreeDecomposition::NodeType::Leaf:
            return c[t][f] = 0;
        case NiceTreeDecomposition::NodeType::IntroduceVertex: {
            int pos = bag_pos(node.bag, node.v);
            Color f_v = at(f, pos);
            // This vertex could already be dominated by some reduction rule.
            if (f_v == Color::WHITE && !g.isDominated(node.v))
                return c[t][f] = INF;
            else {
                return c[t][f] = getC(node.l_child, cut(f, pos));
            }
        }
        case NiceTreeDecomposition::NodeType::IntroduceEdge: {
            int pos_u = bag_pos(node.bag, node.to);
            int pos_v = bag_pos(node.bag, node.v);

            Color f_u = at(f, pos_u);
            Color f_v = at(f, pos_v);

            EdgeStatus edge_status = g.getEdgeStatus(node.to, node.v);
            DS_ASSERT(edge_status == EdgeStatus::UNCONSTRAINED ||
                      edge_status == EdgeStatus::FORCED);
            if (edge_status == EdgeStatus::FORCED) {
                // We are forced to take at least one of the endpoints of the edge to the
                // dominating set.
                if (f_u == Color::BLACK && f_v == Color::WHITE)
                    return c[t][f] = getC(node.l_child, set(f, pos_v, Color::GRAY));
                else if (f_u == Color::WHITE && f_v == Color::BLACK)
                    return c[t][f] = getC(node.l_child, set(f, pos_u, Color::GRAY));
                else if (f_u == Color::BLACK || f_v == Color::BLACK)
                    return c[t][f] = getC(node.l_child, f);
                else
                    return c[t][f] = INF;
            } else {
                if (f_u == Color::BLACK && f_v == Color::WHITE)
                    return c[t][f] = getC(node.l_child, set(f, pos_v, Color::GRAY));
                else if (f_u == Color::WHITE && f_v == Color::BLACK)
                    return c[t][f] = getC(node.l_child, set(f, pos_u, Color::GRAY));
                else
                    return c[t][f] = getC(node.l_child, f);
            }
        }
        case NiceTreeDecomposition::NodeType::Forget: {
            int pos_w = bag_pos(td[node.l_child].bag, node.v);
            return c[t][f] = std::min(
                       cost(g, node.v) + getC(node.l_child, insert(f, pos_w, Color::BLACK)),
                       getC(node.l_child, insert(f, pos_w, Color::WHITE)));
        }
        case NiceTreeDecomposition::NodeType::Join: {
#if DS_BENCHMARK
            auto start = std::chrono::high_resolution_clock::now();
#endif
            int zeros = 0;
            size_t N = node.bag.size();
            for (size_t i = 0; i < N; ++i) {
                if (at(f, i) == Color::WHITE)
                    zeros++;
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
                c[t][f] = std::min(c[t][f],
                                   getC(node.l_child, f_1) + getC(node.r_child, f_2));
            }
            return c[t][f];
        }
    }

    throw std::logic_error("Unknown node type reached in calc_c!");
}

void TreewidthSolver::recoverDS(int t, TernaryFun f) {
    auto &node = td[t];
    DS_ASSERT(f < pow3[node.bag.size()]);
    DS_ASSERT(!c[t].empty() && c[t][f] != UNSET);

    auto bag_pos = [&](const std::vector<int> &bag, int v) -> int {
        int pos = 0;
        while (bag[pos] != v) ++pos;
        return pos;
    };

    switch (node.type) {
        case NiceTreeDecomposition::NodeType::IntroduceVertex: {
            int pos = bag_pos(node.bag, node.v);
            recoverDS(node.l_child, cut(f, pos));
            return;
        }
        case NiceTreeDecomposition::NodeType::IntroduceEdge: {
            int pos_u = bag_pos(node.bag, node.to);
            int pos_v = bag_pos(node.bag, node.v);

            Color f_u = at(f, pos_u);
            Color f_v = at(f, pos_v);

            EdgeStatus edge_status = g.getEdgeStatus(node.to, node.v);
            DS_ASSERT(edge_status == EdgeStatus::UNCONSTRAINED ||
                      edge_status == EdgeStatus::FORCED);
            if (edge_status == EdgeStatus::FORCED) {
                if (f_u == Color::BLACK && f_v == Color::WHITE)
                    recoverDS(node.l_child, set(f, pos_v, Color::GRAY));
                else if (f_u == Color::WHITE && f_v == Color::BLACK)
                    recoverDS(node.l_child, set(f, pos_u, Color::GRAY));
                else if (f_u == Color::BLACK || f_v == Color::BLACK)
                    recoverDS(node.l_child, f);
                else
                    throw std::logic_error(
                        "entered IntroduceEdge state corresponding to no solution");
            } else {
                if (f_u == Color::BLACK && f_v == Color::WHITE)
                    recoverDS(node.l_child, set(f, pos_v, Color::GRAY));
                else if (f_u == Color::WHITE && f_v == Color::BLACK)
                    recoverDS(node.l_child, set(f, pos_u, Color::GRAY));
                else
                    recoverDS(node.l_child, f);
            }
            return;
        }
        case NiceTreeDecomposition::NodeType::Forget: {
            int pos_w = bag_pos(td[node.l_child].bag, node.v);
            if (c[t][f] ==
                cost(g, node.v) + getC(node.l_child, insert(f, pos_w, Color::BLACK))) {
                g.ds.push_back(node.v);
                recoverDS(node.l_child, insert(f, pos_w, Color::BLACK));
            } else {
                recoverDS(node.l_child, insert(f, pos_w, Color::WHITE));
            }
            return;
        }
        case NiceTreeDecomposition::NodeType::Join: {
            int zeros = 0;
            size_t N = node.bag.size();
            for (size_t i = 0; i < N; ++i) {
                if (at(f, i) == Color::WHITE)
                    zeros++;
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

                if (c[t][f] == getC(node.l_child, f_1) + getC(node.r_child, f_2)) {
                    recoverDS(node.l_child, f_1);
                    recoverDS(node.r_child, f_2);
                    return;
                }
            }

            throw std::logic_error("encountered invalid join state");
        }
        default:
            return;
    }
}

uint64_t TreewidthSolver::getMemoryUsage(const NiceTreeDecomposition &td) {
    uint64_t res = 0;
    for (int i = 0; i < td.n_nodes(); i++) {
        res += pow3[td[i].bag.size()] * sizeof(int) + sizeof(std::vector<int>);
    }

    return res;
}

}  // namespace DSHunter