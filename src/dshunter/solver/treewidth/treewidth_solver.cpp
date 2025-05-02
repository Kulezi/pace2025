#include "treewidth_solver.h"

#include <format>
#include <memory>

#include "../../utils.h"
#include "td/exec_decomposer.h"
#include "td/flow_cutter_decomposer.h"

namespace {
std::unique_ptr<DSHunter::Decomposer> getDecomposer(const DSHunter::SolverConfig *cfg) {
    if (cfg->decomposer_path.empty())
        return std::make_unique<DSHunter::FlowCutterDecomposer>(cfg);
    return std::make_unique<DSHunter::ExecDecomposer>(cfg);
}

uint64_t getMemoryUsage(const DSHunter::NiceTreeDecomposition &td) {
    uint64_t res = 0;
    for (int i = 0; i < td.n_nodes(); i++) {
        res += DSHunter::pow3[td[i].bag_size] * sizeof(int) + sizeof(std::vector<int>);
    }

    return res;
}

constexpr int UNSET = -1, INF = 1'000'000'000;

}  // namespace

namespace DSHunter {
TreewidthSolver::TreewidthSolver(SolverConfig *cfg) : cfg(cfg), decomposer(getDecomposer(cfg)) {}

// Returns true if instance was solved. Solution set is stored in given instance.
// Returns false if it would lead to exceeding the memory limit.
bool TreewidthSolver::solve(Instance &instance) {
    auto o = decomposer->decompose(instance);
    if (!o.has_value()) {
        cfg->logLine("decomposition failed");
        return false;
    }

    auto td = o.value();
    cfg->logLine("best found decomposition width: " + std::to_string(td.width));
    if (td.width > cfg->good_enough_treewidth) {
        cfg->logLine(std::format("decomposition width > {}, considering reducing it with bag-branching of depth at most {}", cfg->good_enough_treewidth, cfg->max_bag_branch_depth));
        auto estimate = estimateBranching(instance, td);
        if (estimate.depth_needed <= cfg->max_bag_branch_depth) {
            cfg->logLine(std::format("bag-branching of depth at most {} is enough, proceeding with bag-branching", cfg->max_bag_branch_depth));
        } else {
            cfg->logLine(std::format("bag-branching of depth at most {} is not enough, aborting bag-branching", cfg->max_bag_branch_depth));
            return false;
        }

        solved_leaves = 0;
        total_leaves = estimate.leaves;
        return solveBranching(instance, td);
    }

    return solveDecomp(instance, td);
}

bool TreewidthSolver::solveDecomp(Instance &instance, TreeDecomposition raw_td) {
    g = instance;
    td = NiceTreeDecomposition::nicify(g, raw_td);
    cfg->logLine(std::format("solving td({})", td.width()));
    if (getMemoryUsage(td) > cfg->max_memory_in_bytes) {
        return false;
    }

    c = std::vector<std::vector<int>>(td.n_nodes(), std::vector<int>());

    getC(td.root, 0);
    recoverDS(td.root, 0);
    instance.ds = g.ds;
    cfg->logLine(std::format("found solution of size {}", g.ds.size()));
    return true;
}

TreewidthSolver::BranchingEstimate TreewidthSolver::estimateBranching(Instance &instance, TreeDecomposition td, int depth) {
    int biggest_bag = td.biggestBag();
    int join_tw = 0;

    const int cutoff = cfg->good_enough_treewidth;
    std::vector<int> important_bags;
    for (int i = 0; i < td.size(); i++) {
        if ((int)td.bag[i].size() > cutoff && td.adj[i].size() > 2) {
            important_bags.push_back(i);
            join_tw = std::max(join_tw, (int)td.bag[i].size());
        }
    }

    std::vector<int> counts(instance.all_nodes.size(), 0);
    for (auto bag : important_bags) {
        for (auto v : td.bag[bag]) counts[v]++;
    }

    if (join_tw <= cutoff)
        return { depth, 1 };

    if (depth == cfg->max_bag_branch_depth) {
        return { INF, 1 };
    }

    DS_ASSERT(!td.bag[biggest_bag].empty());
    int v = td.bag[biggest_bag][0];
    for (auto u : td.bag[biggest_bag]) {
        if (counts[v] < counts[u] || (counts[v] == counts[u] && instance.deg(v) > instance.deg(u)))
            v = u;
    }

    std::vector<int> best_ds;

    BranchingEstimate total_estimate{ 0, 0 };
    for (auto taken : instance.neighbourhoodIncluding(v)) {
        auto new_instance = instance;
        auto new_td = td;
        new_instance.take(taken);
        new_td.removeNode(taken);

        if (taken != v) {
            new_instance.removeNode(v);
            new_td.removeNode(v);
        }

        auto estimate = estimateBranching(new_instance, new_td, depth + 1);
        if (estimate.depth_needed >= INF)
            return estimate;

        total_estimate.depth_needed = std::max(total_estimate.depth_needed, estimate.depth_needed);
        total_estimate.leaves += estimate.leaves;
    }

    return total_estimate;
}

bool TreewidthSolver::solveBranching(Instance &instance, TreeDecomposition td) {
    int biggest_bag = td.biggestBag();
    int join_tw = 0;

    const int cutoff = cfg->good_enough_treewidth;
    std::vector<int> important_bags;
    for (int i = 0; i < td.size(); i++) {
        if ((int)td.bag[i].size() > cutoff && td.adj[i].size() > 2) {
            important_bags.push_back(i);
            join_tw = std::max(join_tw, (int)td.bag[i].size());
        }
    }

    std::vector<int> counts(instance.all_nodes.size(), 0);
    for (auto bag : important_bags) {
        for (auto v : td.bag[bag]) counts[v]++;
    }

    if (join_tw <= cutoff) {
        cfg->logLine(std::format("branch {}/{}", solved_leaves, total_leaves));
        solved_leaves++;
        return solveDecomp(instance, td);
    }

    DS_ASSERT(!td.bag[biggest_bag].empty());
    int v = td.bag[biggest_bag][0];
    for (auto u : td.bag[biggest_bag]) {
        if (counts[v] < counts[u] || (counts[v] == counts[u] && instance.deg(v) > instance.deg(u)))
            v = u;
    }

    std::vector<int> best_ds;
    for (auto taken : instance.neighbourhoodIncluding(v)) {
        if (g.isDisregarded(taken))
            continue;
        auto new_instance = instance;
        auto new_td = td;
        new_instance.take(taken);
        new_td.removeNode(taken);

        if (taken != v) {
            auto adj = g[v].adj;
            for (auto [u, s] : adj) {
                if (s == EdgeStatus::FORCED) {
                    g.take(u);
                    new_td.removeNode(u);
                }
            }
            new_instance.removeNode(v);
            new_td.removeNode(v);
        }

        if (!solveBranching(new_instance, new_td)) {
            best_ds = {};
            break;
        }

        if (best_ds.empty() || best_ds.size() > new_instance.ds.size()) {
            best_ds = new_instance.ds;
        }
    }

    if (best_ds.empty())
        return false;

    instance.ds = best_ds;
    return true;
}

inline int TreewidthSolver::cost(int v) {
    if (g[v].is_extra || g.isDisregarded(v))
        return INF;
    return 1;
}

// [Parameterized Algorithms [7.3.2] - 10.1007/978-3-319-21275-3] extended to handle forced
// edges.
int TreewidthSolver::getC(int t, TernaryFun f) {
    const auto &node = td[t];
    DS_ASSERT(f < pow3[node.bag_size]);

    if (!c[t].empty() && c[t][f] != UNSET)
        return c[t][f];
    if (c[t].empty())
        c[t].resize(pow3[node.bag_size], UNSET);
    c[t][f] = INF;

    switch (node.type) {
        case NiceTreeDecomposition::NodeType::Leaf:
            return c[t][f] = 0;
        case NiceTreeDecomposition::NodeType::IntroduceVertex: {
            int pos = node.pos_v;
            Color f_v = at(f, pos);
            // This vertex could already be dominated by some reduction rule.
            if (f_v == Color::WHITE && !g.isDominated(node.v))
                return c[t][f] = INF;
            else {
                return c[t][f] = getC(node.l_child, cut(f, pos));
            }
        }
        case NiceTreeDecomposition::NodeType::IntroduceEdge: {
            int pos_u = node.pos_to;
            int pos_v = node.pos_v;

            Color f_u = at(f, pos_u);
            Color f_v = at(f, pos_v);

            EdgeStatus edge_status = g.getEdgeStatus(node.to, node.v);
            DS_ASSERT(edge_status == EdgeStatus::UNCONSTRAINED ||
                      edge_status == EdgeStatus::FORCED);
            if (edge_status == EdgeStatus::FORCED) {
                // We are forced to take at least one of the endpoints of the edge to the
                // dominating set.
                if (f_u == Color::BLACK && f_v == Color::WHITE)
                    return c[t][f] = getC(node.l_child, setUnset(f, pos_v, Color::GRAY));
                else if (f_u == Color::WHITE && f_v == Color::BLACK)
                    return c[t][f] = getC(node.l_child, setUnset(f, pos_u, Color::GRAY));
                else if (f_u == Color::BLACK || f_v == Color::BLACK)
                    return c[t][f] = getC(node.l_child, f);
                else
                    return c[t][f] = INF;
            } else {
                if (f_u == Color::BLACK && f_v == Color::WHITE)
                    return c[t][f] = getC(node.l_child, setUnset(f, pos_v, Color::GRAY));
                else if (f_u == Color::WHITE && f_v == Color::BLACK)
                    return c[t][f] = getC(node.l_child, setUnset(f, pos_u, Color::GRAY));
                else
                    return c[t][f] = getC(node.l_child, f);
            }
        }
        case NiceTreeDecomposition::NodeType::Forget: {
            int pos_v = node.pos_v;
            int cost_take = cost(node.v);
            // Skip the branching if we already know the solution would be unoptimal.
            if (cost_take < INF) {
                return c[t][f] = std::min(
                           cost_take + getC(node.l_child, insert(f, pos_v, Color::BLACK)),
                           getC(node.l_child, insert(f, pos_v, Color::WHITE)));
            }

            return c[t][f] = getC(node.l_child, insert(f, pos_v, Color::WHITE));
        }
        case NiceTreeDecomposition::NodeType::Join: {
            size_t N = node.bag_size;
            std::vector<int> zeroes;
            for (size_t i = 0; i < N; ++i) {
                if (at(f, i) == Color::WHITE)
                    zeroes.push_back(i);
            }

            // Iterate over all combinations of choosing f_1(v), f_2(v) for positions where
            // f(v) = 0.
            for (int mask = 0; mask < (1 << zeroes.size()); mask++) {
                // The value of f_1, f_2 will be the same on all trits that ain't 0 in f, so
                // we don't need to touch those.
                TernaryFun f_1 = f, f_2 = f;
                for (size_t i = 0; i < zeroes.size(); ++i) {
                    if (mask >> i & 1) {
                        f_1 = setUnset(f_1, zeroes[i], Color::GRAY);
                    } else {
                        f_2 = setUnset(f_2, zeroes[i], Color::GRAY);
                    }
                }
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
    DS_ASSERT(f < pow3[node.bag_size]);
    DS_ASSERT(!c[t].empty() && c[t][f] != UNSET);
    switch (node.type) {
        case NiceTreeDecomposition::NodeType::IntroduceVertex: {
            int pos = node.pos_v;
            recoverDS(node.l_child, cut(f, pos));
            return;
        }
        case NiceTreeDecomposition::NodeType::IntroduceEdge: {
            int pos_u = node.pos_to;
            int pos_v = node.pos_v;

            Color f_u = at(f, pos_u);
            Color f_v = at(f, pos_v);

            EdgeStatus edge_status = g.getEdgeStatus(node.to, node.v);
            DS_ASSERT(edge_status == EdgeStatus::UNCONSTRAINED ||
                      edge_status == EdgeStatus::FORCED);
            if (edge_status == EdgeStatus::FORCED) {
                if (f_u == Color::BLACK && f_v == Color::WHITE)
                    recoverDS(node.l_child, setUnset(f, pos_v, Color::GRAY));
                else if (f_u == Color::WHITE && f_v == Color::BLACK)
                    recoverDS(node.l_child, setUnset(f, pos_u, Color::GRAY));
                else if (f_u == Color::BLACK || f_v == Color::BLACK)
                    recoverDS(node.l_child, f);
                else
                    throw std::logic_error(
                        "entered IntroduceEdge state corresponding to no solution");
            } else {
                if (f_u == Color::BLACK && f_v == Color::WHITE)
                    recoverDS(node.l_child, setUnset(f, pos_v, Color::GRAY));
                else if (f_u == Color::WHITE && f_v == Color::BLACK)
                    recoverDS(node.l_child, setUnset(f, pos_u, Color::GRAY));
                else
                    recoverDS(node.l_child, f);
            }
            return;
        }
        case NiceTreeDecomposition::NodeType::Forget: {
            int pos_v = node.pos_v;
            if (c[t][f] ==
                cost(node.v) + getC(node.l_child, insert(f, pos_v, Color::BLACK))) {
                g.ds.push_back(node.v);
                recoverDS(node.l_child, insert(f, pos_v, Color::BLACK));
            } else {
                recoverDS(node.l_child, insert(f, pos_v, Color::WHITE));
            }
            return;
        }
        case NiceTreeDecomposition::NodeType::Join: {
            size_t N = node.bag_size;
            std::vector<int> zeroes;
            for (size_t i = 0; i < N; ++i) {
                if (at(f, i) == Color::WHITE)
                    zeroes.push_back(i);
            }

            // Iterate over all combinations of choosing f_1(v), f_2(v) for positions where
            // f(v) = 0.
            for (int mask = 0; mask < (1 << zeroes.size()); mask++) {
                // The value of f_1, f_2 will be the same on all trits that ain't 0 in f, so
                // we don't need to touch those.
                TernaryFun f_1 = f, f_2 = f;
                for (size_t i = 0; i < zeroes.size(); ++i) {
                    if (mask >> i & 1) {
                        f_1 = setUnset(f_1, zeroes[i], Color::GRAY);
                    } else {
                        f_2 = setUnset(f_2, zeroes[i], Color::GRAY);
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

}  // namespace DSHunter