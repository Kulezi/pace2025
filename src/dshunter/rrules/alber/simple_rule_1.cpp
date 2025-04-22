#include "../rrules.h"
#include <set>
namespace DSHunter {
bool alberSimpleRule1(Instance& g) {
    // set instead of vector as there might be a case 1-2-3, where 1 and 3 are disregarded, we don't want to take twice.
    std::set<int> to_take;
    std::vector<std::pair<int,int>> to_remove;
    for (auto v : g.nodes) {
        for (auto [w, status] : g[v].adj) {
            if (v < w && status == EdgeStatus::UNCONSTRAINED && g.isDominated(v) && g.isDominated(w)) {
                to_remove.emplace_back(v, w);
            }

            if (v < w && g.isDisregarded(v) && g.isDisregarded(w)) {
                DS_ASSERT(status != EdgeStatus::FORCED);
                to_remove.emplace_back(v, w);
            }

            if (status == EdgeStatus::FORCED && g.isDisregarded(v)) {
                DS_ASSERT(!g.isDisregarded(w));
                to_take.insert(w);
            }
        }
    }

    for (auto [v, w] : to_remove) {
        g.removeEdge(v, w);
    }

    for (auto v : to_take) {
        g.take(v);
    }

    return !to_remove.empty() || !to_take.empty();
}

ReductionRule AlberSimpleRule1("AlberSimpleRule1 (dominated edge removal)", alberSimpleRule1, 2, 1);

}  // namespace DSHunter