#include "../rrules.h"

namespace {
    bool allNeighboursAreDominated(DSHunter::Instance &g, int v) {
        for (auto u : g[v].n_open) {
            if (!g.isDominated(u)) return false;
        }
        return true;
    }
}

namespace DSHunter {

bool hittingSetRule(Instance& g) {
    auto nodes = g.nodes;
    bool reduced = false;

    for (auto v : nodes) {
        if (!g.isDominated(v) && allNeighboursAreDominated(g, v)) {
            g.sets_to_hit.push_back(g[v].n_open);
            g.removeNode(v);
        }
    }

    return reduced;
}

ReductionRule HittingSetRule("HittingSetRule", hittingSetRule, 1, 1);

}  // namespace DSHunter