#include "../rrules.h"
namespace DSHunter {
bool alberSimpleRule1(Instance& g) {
    for (auto v : g.nodes) {
        for (auto [w, status] : g[v].adj) {
            if (status == EdgeStatus::UNCONSTRAINED && g.isDominated(v) && g.isDominated(w)) {
                g.removeEdge(v, w);
                return true;
            }

            if (g.isDisregarded(v) && g.isDisregarded(w)) {
                DS_ASSERT(status != EdgeStatus::FORCED);
                g.removeEdge(v, w);
                return true;
            }

            if (status == EdgeStatus::FORCED && g.isDisregarded(v)) {
                DS_ASSERT(!g.isDisregarded(w));
                g.take(w);
                return true;
            }
        }
    }

    return false;
}

ReductionRule AlberSimpleRule1("AlberSimpleRule1 (dominated edge removal)", alberSimpleRule1, 2, 1);

}  // namespace DSHunter