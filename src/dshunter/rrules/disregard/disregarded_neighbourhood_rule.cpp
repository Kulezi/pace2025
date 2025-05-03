#include "../rrules.h"
namespace {
bool allNeighboursArePink(DSHunter::Instance& g, int u) {
    for (auto v : g[u].n_open) {
        if (!g.isDisregarded(v))
            return false;
    }
    return true;
}
}  // namespace
namespace DSHunter {

bool disregardedNeighbourhoodRule(Instance& g) {
    auto nodes = g.nodes;
    bool reduced = false;
    for (auto u : nodes) {
        if (g.hasNode(u) && !g.isDominated(u) && !g.isDisregarded(u) && allNeighboursArePink(g, u)) {
            g.take(u);
            reduced = true;
        }
    }

    return reduced;
}

ReductionRule DisregardedNeighbourhoodRule("DisregardedNeighbourhoodRule", disregardedNeighbourhoodRule, 2, 1);

}  // namespace DSHunter