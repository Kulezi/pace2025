#include "../rrules.h"
namespace DSHunter {
bool alberSimpleRule2(Instance& g) {
    std::vector<int> to_remove;
    std::vector<int> to_take;
    for (auto v : g.nodes) {
        if (g.isDominated(v) && g.deg(v) <= 1) {
            to_remove.push_back(v);
            if (g.deg(v) == 1) {
                auto [w, status] = g[v].adj[0];
                // The edge is forced so it's optimal to take the end that possibly could have
                // larger degree. If the other end of the edge also would be a candidate for
                // this reduction, apply it only to the vertex with smaller label.
                if (status == EdgeStatus::FORCED && !(g.deg(w) == 1 && g.isDominated(w) && v > w)) {
                    to_take.push_back(w);
                }
            }
        }
    }

    for (auto v : to_take) {
        if (!g.isTaken(v)) {
            DS_TRACE(std::cerr << "applying " << __func__ << " (take) " << dbg(v) << std::endl);
            g.take(v);
        }
    }

    for (auto v : to_remove) {
        DS_TRACE(std::cerr << "applying " << __func__ << " (remove) " << dbg(v) << std::endl);
        g.removeNode(v);
    }

    return !to_remove.empty() || !to_take.empty();
}

ReductionRule AlberSimpleRule2("AlberSimpleRule2 (dominated leaf removal)", alberSimpleRule2, 2, 1);

}  // namespace DSHunter