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
    std::vector<int> to_take;
    for (auto u : g.nodes) {
        if (!g.isDominated(u) && !g.isDisregarded(u) && allNeighboursArePink(g, u)) {
            to_take.push_back(u);
        }
    }

    for (auto u : to_take) {
        g.take(u);
    }

    return !to_take.empty();
}

ReductionRule DisregardedNeighbourhoodRule("DisregardedNeighbourhoodRule", disregardedNeighbourhoodRule, 2, 1);

}  // namespace DSHunter