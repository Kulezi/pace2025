#include "branching_solver.h"

#include <limits.h>

#include <vector>
#include <cstdint>

#include "../../bounds.h"
#include "../../instance.h"
#include "../../rrules/rrules.h"
#include "../../utils.h"

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

constexpr int MAX_RULE_COMPLEXITY = 3;

Instance take(Instance g, int v) {
    g.take(v);
    DS_TRACE(std::cerr << std::string(level, ' ') << "took" << dbg(v) << dbgv(g.ds) << std::endl);
    return g;
}

Instance take(Instance g, std::vector<int> to_take) {
    for (auto v : to_take) g.take(v);
    DS_TRACE(std::cerr << std::string(level, ' ') << "took" << dbgv(to_take) << dbgv(g.ds)
                       << std::endl);
    return g;
}

int undominatedDegree(const Instance &g, int v) {
    int d = !g.isDominated(v);
    for (auto [w, _] : g[v].adj) {
        if (!g.isDominated(w))
            d++;
    }

    return d;
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

BranchingSolver::BranchingSolver() : reduction_rules(get_default_reduction_rules()) {}
BranchingSolver::BranchingSolver(std::vector<ReductionRule> rrules) : reduction_rules(rrules) {}

void BranchingSolver::solve(Instance g, std::vector<int> &best_ds) {
    enter;

    reduce(g, reduction_rules, 3);
    if (!best_ds.empty() && g.ds.size() + DSHunter::lower_bound(g) >= best_ds.size()) {
        leave;
    }

    int v = selectNode(g);

    if (v == -1) {
        std::cerr << "best found: " << g.ds.size() << std::endl;
        best_ds = g.ds;
        leave;
    }

    branch(g, v, best_ds);
    leave;
}

int BranchingSolver::selectNode(const Instance &g) {
    int v = maxForcedDegreeNode(g);
    if (v != -1)
        return v;
    return maxUndominatedDegreeNode(g);
}

void BranchingSolver::branch(const Instance &g, int v, std::vector<int> &best_ds) {
    // Case 1: v belongs to DS -> dominate N(v)
    // In case it is a leaf we can skip this case since taking the other end is always at least as
    // good.
    if (g.deg(v) != 1)
        solve(take(g, v), best_ds);

    std::vector<int> to_take;
    for (auto [u, s] : g[v].adj)
        if (s == EdgeStatus::FORCED)
            to_take.push_back(u);

    if (to_take.empty()) {
        // Since v doesn't belong to DS we need to take some neighbour to the
        // dominating set.

        // TODO: When taking i-th neighbour we can remove neigbours 1, ..., i - 1, since we already
        // handled cases where they are in the dominating set in the loop.
        for (auto [u, s] : g[v].adj) {
            DS_ASSERT(s == EdgeStatus::UNCONSTRAINED);
            solve(take(g, u), best_ds);
        }
    }
    // Case 2.2: v has forced edges, and we don't take v to the dominating set.
    // In this case we need to take all its forced-edge connected neighbours into the dominating
    // set.
    else
        solve(take(g, to_take), best_ds);
}

}  // namespace DSHunter