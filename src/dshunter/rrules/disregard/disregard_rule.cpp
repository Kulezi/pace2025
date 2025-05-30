#include "../rrules.h"
namespace {

bool hasRedEdge(DSHunter::Instance& g, const int u, const int excluded) {
    return std::ranges::any_of(g[u].adj, [&](const DSHunter::Endpoint e) {
        return e.to != excluded && e.status == DSHunter::EdgeStatus::FORCED;
    });
}

}  // namespace

namespace DSHunter {

bool disregardRule(Instance& g) {
    bool marked = false;
    for (const auto u : g.nodes) {
        for (auto [v, s] : g[u].adj) {
            if (g[v].membership_status != MembershipStatus::DISREGARDED &&
                g[u].membership_status == MembershipStatus::UNDECIDED &&
                contains(g[v].dominatees, g[u].dominatees) &&
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