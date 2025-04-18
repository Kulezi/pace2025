#include "../rrules.h"
namespace {
bool allNeighboursArePink(DSHunter::Instance& g, int u) {
    for (auto [v, _] : g[u].adj) {
        if (!g.isDisregarded(v))
            return false;
    }
    return true;
}
}  // namespace
namespace DSHunter {

bool disregardedNeighbourhoodRule(Instance& g) {
    for (auto u : g.nodes) {
        if (!g.isDisregarded(u) && allNeighboursArePink(g, u)) {
            g.take(u);
            return true;
        }
    }

    return false;
}

ReductionRule DisregardedNeighbourhoodRule("DisregardedNeighbourhoodRule", disregardedNeighbourhoodRule, 2, 1);

}  // namespace DSHunter