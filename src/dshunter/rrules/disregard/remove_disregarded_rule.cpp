#include "../rrules.h"

namespace DSHunter {

bool removeDisregardedRule(Instance& g) {
    for (auto u : g.nodes) {
        if (g.isDisregarded(u) && g.isDominated(u)) {
            for (auto [v, s] : g[u].adj)
                if (s == EdgeStatus::FORCED)
                    g.take(v);
            g.removeNode(u);
            return true;
        }
    }

    return false;
}

ReductionRule RemoveDisregardedRule("RemoveDisregardedRule", removeDisregardedRule, 2, 1);

}  // namespace DSHunter