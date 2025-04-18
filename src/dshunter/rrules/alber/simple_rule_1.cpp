#include "../rrules.h"
namespace DSHunter {
bool alberSimpleRule1(Instance& g) {
    std::vector<std::pair<int, int>> to_remove;
    for (auto v : g.nodes) {
        for (auto [w, status] : g[v].adj) {
            if (v > w || status == EdgeStatus::FORCED) continue;
            if (g.isDominated(v) && g.isDominated(w)) {
                to_remove.emplace_back(v, w);
            }
        }
    }
    for (auto [v, w] : to_remove) {
        DS_TRACE(std::cerr << "applying " << __func__ << dbg(v) << dbg(w) << std::endl);
        g.removeEdge(v, w);
    }

    return !to_remove.empty();
}

ReductionRule AlberSimpleRule1("AlberSimpleRule1 (dominated edge removal)", alberSimpleRule1, 2, 1);

}  // namespace DSHunter