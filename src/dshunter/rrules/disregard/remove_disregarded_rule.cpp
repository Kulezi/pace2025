
#include "../rrules.h"

namespace DSHunter {

bool removeDisregardedRule(Instance& g) {
    for (auto u : g.nodes) {
        if (g.isDisregarded(u) && g.isDominated(u)) {
            g.removeNode(u);
            return true;
        }
    }

    return false;
}

ReductionRule RemoveDisregardedRule("RemoveDisregardedRule", removeDisregardedRule, 2, 1);

}  // namespace DSHunter