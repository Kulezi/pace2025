#include "../rrules.h"
namespace {

}  // namespace
namespace DSHunter {

bool sameDominatorsRule(Instance& g) {
    bool reduced = false;
    for (auto u : g.nodes) {
        if (g.isDominated(u))
            continue;

        for (auto v : g[u].n_open) {
            if (contains(g[v].dominators, g[u].dominators)) {
                g.markDominated(u);
                reduced = true;
                break;
            }
        }
    }

    return reduced;
}

ReductionRule SameDominatorsRule("SameDominatorsRule", sameDominatorsRule, 2, 1);

}  // namespace DSHunter