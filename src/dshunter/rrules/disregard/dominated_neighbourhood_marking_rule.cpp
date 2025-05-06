#include "../rrules.h"
namespace {
bool allNeighboursAreDominated(DSHunter::Instance& g, int u) {
    for (auto v : g[u].n_open) {
        if (!g.isDominated(v))
            return false;
    }

    return true;
}

bool atLeastOneNeighbourCanBeTaken(DSHunter::Instance& g, int u) {
    for (auto v : g[u].n_open) {
        if (!g.isDisregarded(v))
            return true;
    }

    return false;
}

bool otherEndCanBeTaken(DSHunter::Instance& g, int u) {
    DS_ASSERT(g.forcedDeg(u) == 1);
    for (auto [v, s] : g[u].adj) {
        if (s == DSHunter::EdgeStatus::FORCED)
            return !g.isDisregarded(v);
    }

    throw std::logic_error("this should be unreachable");
}
}  // namespace
namespace DSHunter {

bool dominatedNeighbourhoodMarkingRule(Instance& g) {
    bool reduced = false;
    for (auto u : g.nodes) {
        if (!g.isDisregarded(u) && allNeighboursAreDominated(g, u) && atLeastOneNeighbourCanBeTaken(g, u)) {
            if (g.forcedDeg(u) == 0 || (g.forcedDeg(u) == 1 && otherEndCanBeTaken(g, u))) {
                g.markDisregarded(u);
                reduced = true;
            }
        }
    }

    return reduced;
}

ReductionRule DominatedNeighbourhoodMarkingRule("DominatedNeighbourhoodMarkingRule", dominatedNeighbourhoodMarkingRule, 2, 1);

}  // namespace DSHunter