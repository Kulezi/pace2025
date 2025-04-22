#include "../rrules.h"

namespace DSHunter {

bool removeDisregardedRule(Instance& g) {
    std::vector<int> to_take;
    for (auto u : g.nodes) {
        if (g.isDisregarded(u) && g.isDominated(u)) {
            to_take.push_back(u);
        }
    }

    for (auto u : to_take) {
        for (auto [v, s] : g[u].adj) {
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