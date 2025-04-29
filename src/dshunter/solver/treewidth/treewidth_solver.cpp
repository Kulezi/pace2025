#include "treewidth_solver.h"

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
}  // namespace

namespace DSHunter {
TreewidthSolver::TreewidthSolver(SolverConfig *cfg) : cfg(cfg), decomposer(getDecomposer(cfg)) {}

constexpr int UNSET = -1, INF = 1'000'000'000;

// Returns true if instance was solved. Solution set is stored in given instance.
// Returns false if it would lead to exceeding the memory limit.
bool TreewidthSolver::solve(Instance &instance) {
    auto o = decomposer->decompose(instance);
    if (!o.has_value()) {
        std::cerr << "Decomposition failed" << std::endl;
        return false;
    }

    auto td = o.value();
    std::cerr << "Best found decomposition width: " << td.width << std::endl;
    if (td.width > 14) {
        std::cerr << "Decomposition width too big, considering bag-branching" << std::endl;
        return solveJoinKiller(instance, td, cfg->max_bag_branch_depth);
    }

    return solveDecomp(instance, td);
}

bool TreewidthSolver::solveDecomp(Instance &instance, TreeDecomposition raw_td) {
    g = instance;
    td = NiceTreeDecomposition::nicify(g, raw_td);
    std::cerr << "solving td(" << td.width();
    if (getMemoryUsage(td) > cfg->max_memory_in_bytes) {
        std::cerr << ") FAIL\n";
        return false;
    }

    c = std::vector<std::vector<int>>(td.n_nodes(), std::vector<int>());

    getC(td.root, 0);
    recoverDS(td.root, 0);
    instance.ds = g.ds;
    std::cerr << ") SUCCESS(" << g.ds.size() << ")\n";
    return true;
}

bool TreewidthSolver::solveBagBranching(Instance &instance, TreeDecomposition td, int remaining_depth) {
    int biggest_bag = td.biggestBag();
    int tw = td.bag[biggest_bag].size();
    for (int i = 0; i < cfg->max_bag_branch_depth - remaining_depth; i++) { std::cerr << " "; }
    std::cerr << tw << "\n";

    if (remaining_depth == 0 || instance.nodeCount() == 0) {
        if (tw > 15) {
            std::cerr << "insufficient depth to reach tw < 15\n";
            return false;
        }
        return solveDecomp(instance, td);
    }

    DS_ASSERT(!td.bag[biggest_bag].empty());
    int v = td.bag[biggest_bag][0];
    for (auto u : td.bag[biggest_bag]) {
        if (instance.deg(u) > instance.deg(v))
            v = u;
    }

    std::cerr << dbg(v) << dbg(instance.deg(v)) << dbg(instance.forcedDeg(v)) << std::endl;
    std::vector<int> best_ds;
    for (auto taken : instance.neighbourhoodIncluding(v)) {
        auto new_instance = instance;
        auto new_td = td;
        new_instance.take(taken);
        new_td.removeNode(taken);

        if (taken != v) {
            new_instance.removeNode(v);
            new_td.removeNode(v);
        }

        auto old = new_instance.nodes;
        reduce(new_instance, cfg->reduction_rules);
        for (auto v : remove(old, new_instance.nodes))
            new_td.removeNode(v);

        if (!solveBagBranching(new_instance, new_td, remaining_depth - 1)) {
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

bool TreewidthSolver::solveBagKiller(Instance &instance, TreeDecomposition td, int remaining_depth) {
    int biggest_bag = td.biggestBag();
    int tw = td.bag[biggest_bag].size();

    int cutoff = 15;
    std::vector<int> important_bags;
    for (int i = 0; i < td.size(); i++) {
        if (td.bag[i].size() > cutoff)
            important_bags.push_back(i);
    }

    std::vector<int> counts(instance.all_nodes.size(), 0);
    std::cerr << dbg(remaining_depth) << " sizes: ";
    for (auto bag : important_bags) {
        std::cerr << td.bag[bag].size() << " ";
        for (auto v : td.bag[bag]) counts[v]++;
    }
    std::cerr << std::endl;

    if (tw <= 15)
        return solveDecomp(instance, td);

    if (remaining_depth == 0) {
        std::cerr << "insufficient depth to reach tw < 15\n";
        return false;
    }

    DS_ASSERT(!td.bag[biggest_bag].empty());
    int v = td.bag[biggest_bag][0];
    for (auto u : td.bag[biggest_bag]) {
        if (counts[v] < counts[u] || (counts[v] == counts[u] && instance.deg(v) > instance.deg(u)))
            v = u;
    }

    std::cerr << dbg(v) << dbg(instance.deg(v)) << dbg(instance.forcedDeg(v)) << dbg(counts[v]) << std::endl;
    std::vector<int> best_ds;
    for (auto taken : instance.neighbourhoodIncluding(v)) {
        auto new_instance = instance;
        auto new_td = td;
        new_instance.take(taken);
        new_td.removeNode(taken);

        if (taken != v) {
            new_instance.removeNode(v);
            new_td.removeNode(v);
        }

        auto old = new_instance.nodes;
        reduce(new_instance, cfg->reduction_rules);
        for (auto v : remove(old, new_instance.nodes))
            new_td.removeNode(v);

        if (!solveBagKiller(new_instance, new_td, remaining_depth - 1)) {
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


bool TreewidthSolver::solveJoinKiller(Instance &instance, TreeDecomposition td, int remaining_depth) {
    int biggest_bag = td.biggestBag();
    int tw = td.bag[biggest_bag].size();
    int join_tw = 0;

    int cutoff = 13;
    std::vector<int> important_bags;
    for (int i = 0; i < td.size(); i++) {
        if (td.bag[i].size() > cutoff && td.adj[i].size() > 2) {
            important_bags.push_back(i);
            join_tw = std::max(join_tw, (int)td.bag[i].size());
        }
    }

    std::vector<int> counts(instance.all_nodes.size(), 0);
    std::cerr << dbg(remaining_depth) << " sizes: ";
    for (auto bag : important_bags) {
        std::cerr << td.bag[bag].size() << " ";
        for (auto v : td.bag[bag]) counts[v]++;
    }
    std::cerr << std::endl;

    if (join_tw <= cutoff)
        return solveDecomp(instance, td);

    if (remaining_depth == 0) {
        std::cerr << "insufficient depth to reach tw < 15\n";
        return false;
    }

    DS_ASSERT(!td.bag[biggest_bag].empty());
    int v = td.bag[biggest_bag][0];
    for (auto u : td.bag[biggest_bag]) {
        if (counts[v] < counts[u] || (counts[v] == counts[u] && instance.deg(v) > instance.deg(u)))
            v = u;
    }

    std::cerr << dbg(v) << dbg(instance.deg(v)) << dbg(instance.forcedDeg(v)) << dbg(counts[v]) << std::endl;
    std::vector<int> best_ds;
    for (auto taken : instance.neighbourhoodIncluding(v)) {
        auto new_instance = instance;
        auto new_td = td;
        new_instance.take(taken);
        new_td.removeNode(taken);

        if (taken != v) {
            new_instance.removeNode(v);
            new_td.removeNode(v);
        }

        auto old = new_instance.nodes;
        reduce(new_instance, cfg->reduction_rules);
        for (auto v : remove(old, new_instance.nodes))
            new_td.removeNode(v);

        if (!solveJoinKiller(new_instance, new_td, remaining_depth - 1)) {
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
            int pos_w = bag_pos(td[node.l_child].bag, node.v);
            int cost_take = cost(node.v);
            // Skip the branching if we already know the solution would be unoptimal.
            if (cost_take < INF) {
                return c[t][f] = std::min(
                           cost_take + getC(node.l_child, insert(f, pos_w, Color::BLACK)),
                           getC(node.l_child, insert(f, pos_w, Color::WHITE)));
            }

            return c[t][f] = getC(node.l_child, insert(f, pos_w, Color::WHITE));
        }
        case NiceTreeDecomposition::NodeType::Join: {
            size_t N = node.bag.size();
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
            int pos_w = bag_pos(td[node.l_child].bag, node.v);
            if (c[t][f] ==
                cost(node.v) + getC(node.l_child, insert(f, pos_w, Color::BLACK))) {
                g.ds.push_back(node.v);
                recoverDS(node.l_child, insert(f, pos_w, Color::BLACK));
            } else {
                recoverDS(node.l_child, insert(f, pos_w, Color::WHITE));
            }
            return;
        }
        case NiceTreeDecomposition::NodeType::Join: {
            size_t N = node.bag.size();
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

uint64_t TreewidthSolver::getMemoryUsage(const NiceTreeDecomposition &td) {
    uint64_t res = 0;
    for (int i = 0; i < td.n_nodes(); i++) {
        res += pow3[td[i].bag.size()] * sizeof(int) + sizeof(std::vector<int>);
    }

    return res;
}

}  // namespace DSHunter