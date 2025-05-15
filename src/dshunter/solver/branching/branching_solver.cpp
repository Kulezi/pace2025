#include "branching_solver.h"

#include <climits>
#include <format>
#include <vector>

#include "../../bounds.h"
#include "../../instance.h"
#include "../../rrules/rrules.h"
#include "../../utils.h"
#include "dshunter/solver/heuristic/greedy.h"
#include "dshunter/solver/treewidth/td/flow_cutter_decomposer.h"

namespace {
using DSHunter::Instance;

int level = 0;
uint64_t calls = 0;
#define enter \
    ++level;  \
    ++calls
#define leave \
    --level;  \
    return

Instance take(Instance g, int v) {
    g.take(v);
    DS_TRACE(std::cerr << std::string(level, ' ') << "took" << dbg(v) << dbgv(g.ds) << std::endl);
    return g;
}

Instance take(Instance g, const std::vector<int> &to_take) {
    for (auto v : to_take) g.take(v);
    DS_TRACE(std::cerr << std::string(level, ' ') << "took" << dbgv(to_take) << dbgv(g.ds)
                       << std::endl);
    return g;
}

int undominatedDegree(const Instance &g, int v) {
    return static_cast<int>(g[v].dominatees.size());
}

int maxUndominatedDegreeNode(const Instance &g) {
    int best_deg = 0, best = -1;
    for (auto v : g.nodes) {
        int d = undominatedDegree(g, v);
        if (d > best_deg) {
            best_deg = d;
            best = v;
        }
    }

    return best;
}

int maxForcedDegreeNode(const Instance &g) {
    int best_deg = 0, best = -1;
    for (auto v : g.nodes) {
        int d = g.forcedDeg(v);
        if (d > best_deg) {
            best_deg = d;
            best = v;
        }
    }

    return best;
}
}  // namespace

namespace DSHunter {

BranchingSolver::BranchingSolver(SolverConfig *cfg) : cfg(cfg) {}

std::vector<int> BranchingSolver::solve(const Instance &g) {
    std::vector<int> best_ds = greedyDominatingSet(g);
    solve(g, best_ds);
    return best_ds;
}

void BranchingSolver::solve(Instance g, std::vector<int> &best_ds) {
    enter;

    reduce(g, reduction_rules, cfg->max_branching_reductions_complexity);
    if (lowerBound(g) >= static_cast<int>(best_ds.size()) || !g.isSolvable()) {
        leave;
    }

    auto split = g.split();
    for (auto &cc : split) {
        g.nodes = cc;
        std::vector<int> ds = greedyDominatingSet(g);
        branch(g, ds);
        g.ds = ds;
        if (g.ds.size() >= best_ds.size()) {
            leave;
        }
    }
    best_ds = g.ds;
    leave;
}

int BranchingSolver::selectNode(const Instance &g) {
    int v = maxForcedDegreeNode(g);
    if (v != -1)
        return v;
    return maxUndominatedDegreeNode(g);
}

void BranchingSolver::branch(Instance &g, std::vector<int> &best_ds) {
    int v = selectNode(g);
    if (v == -1) {
        if (g.ds.size() < best_ds.size())
            best_ds = g.ds;
        return;
    }

    if (!g.isDisregarded(v)) {
        solve(take(g, v), best_ds);
        g.markDisregarded(v);
    }

    std::vector<int> to_take;
    for (auto [u, s] : g[v].adj)
        if (s == EdgeStatus::FORCED) {
            if (g.isDisregarded(u))
                return;
            to_take.push_back(u);
        }

    solve(take(g, to_take), best_ds);
}

}  // namespace DSHunter