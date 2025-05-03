#include "../rrules.h"
namespace {

bool hasRedEdge(DSHunter::Instance& g, int u, int excluded) {
    for (auto [w, s] : g[u].adj) {
        if (w != excluded && s == DSHunter::EdgeStatus::FORCED)
            return true;
    }
    return false;
}

}  // namespace

namespace DSHunter {

bool disregardRule(Instance& g) {
    bool marked = false;
    for (auto u : g.nodes) {
        for (auto [v, s] : g[u].adj) {
            if (g[v].membership_status != MembershipStatus::DISREGARDED &&
                g[u].membership_status == MembershipStatus::UNDECIDED &&
                contains(g[v].n_closed, g[u].n_closed) &&
                !hasRedEdge(g, u, v)) {
                g.markDisregarded(u);
                DS_TRACE(std::cerr << "applied DisregardRule to node " << u << std::endl);
                marked = true;
            }
        }
    }

    return marked;
}

ReductionRule DisregardRule("DisregardRule", disregardRule, 2, 1);

}  // namespace DSHunter