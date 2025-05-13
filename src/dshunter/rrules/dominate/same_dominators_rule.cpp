#include "../rrules.h"
namespace {
bool applySameDominatorsRule(DSHunter::Instance& g, int u, int v) {
    if (u != v && !g.isDominated(u) && !g.isDominated(v) && DSHunter::contains(g[u].dominators, g[v].dominators)) {
        g.markDominated(u);
        return true;
    }
    return false;
}

}  // namespace
namespace DSHunter {


bool sameDominatorsRule(Instance& g) {
    bool reduced = false;

    // Iterate over all pairs (u, w) of nodes at distance at most 2.
    for (auto u : g.nodes) {
        for (auto v : g[u].n_open) {
            for (auto w : g[v].n_closed) {
                reduced |= applySameDominatorsRule(g, u, w);
            }
        }
    }

    return reduced;
}

ReductionRule SameDominatorsRule("SameDominatorsRule", sameDominatorsRule, 3, 2);

}  // namespace DSHunter