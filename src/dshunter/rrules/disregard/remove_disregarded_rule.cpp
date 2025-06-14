#include "../rrules.h"

namespace DSHunter {

bool removeDisregardedRule(Instance& g) {
    std::vector<int> to_take;
    for (auto u : g.nodes) {
        if (g.isDisregarded(u) && g.isDominated(u)) {
            to_take.push_back(u);
        }
    }

    for (const auto u : to_take) {
        for (const auto adj = g[u].adj; const auto [v, s] : adj) {
            if (s == EdgeStatus::FORCED) {
                DS_ASSERT(!g.isDisregarded(v));
                g.take(v);
            }
        }
        g.removeNode(u);
    }

    return !to_take.empty();
}

ReductionRule RemoveDisregardedRule("RemoveDisregardedRule", removeDisregardedRule, 2, 1);

}  // namespace DSHunter