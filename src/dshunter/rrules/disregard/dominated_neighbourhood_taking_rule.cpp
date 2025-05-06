#include "../rrules.h"
namespace {
bool allNeighboursAreAtMostDegree2Dominated(DSHunter::Instance& g, int u) {
    for (auto v : g[u].n_open) {
        if (!g.isDominated(v) || g.deg(v) > 2)
            return false;
    }

    return true;
}

bool neighbourToTake(DSHunter::Instance& g, int u) {
    for (auto v : g[u].n_open) {
        if (!g.isDisregarded(v))
            return v;
    }

    return -1;
}

}  // namespace
namespace DSHunter {

bool dominatedNeighbourhoodTakingRule(Instance& g) {
    auto nodes = g.nodes;
    bool reduced = false;
    for (auto u : nodes) {
        if (g.hasNode(u) && g.isDisregarded(u) && allNeighboursAreAtMostDegree2Dominated(g, u)) {
            DS_ASSERT(g.forcedDeg(u) == 0);
            int v = neighbourToTake(g, u);
            if (v != -1) {
                g.removeNode(u);
                g.take(v);
                reduced = true;
            }

        }
    }

    return reduced;
}

ReductionRule DominatedNeighbourhoodTakingRule("DominatedNeighbourhoodTakingRule", dominatedNeighbourhoodTakingRule, 2, 1);

}  // namespace DSHunter