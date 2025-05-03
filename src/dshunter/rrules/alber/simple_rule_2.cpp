#include "../rrules.h"
namespace DSHunter {
bool alberSimpleRule2(Instance& g) {
    auto nodes = g.nodes;
    bool reduced = false;
    
    for (auto v : nodes) {
        if (g.hasNode(v) && g.isDominated(v)) {
            if (g.deg(v) == 0) {
                DS_TRACE(std::cerr << "applying " << __func__ << " (remove) " << dbg(v) << std::endl);
                g.removeNode(v);
                reduced = true;
            } else if (g.deg(v) == 1) {
                auto [w, status] = g[v].adj[0];

                // We might need to use v to dominate w in this case.
                if (g.isDisregarded(w) && !g.isDominated(w))
                    continue;

                // The edge is forced so it's optimal to take the end that possibly could have
                // larger degree. If the other end of the edge also would be a candidate for
                // this reduction, apply it only to the vertex with smaller label.
                if (status == EdgeStatus::FORCED && (g.isDisregarded(v) || !(g.deg(w) == 1 && v > w))) {
                    DS_TRACE(std::cerr << "applying " << __func__ << " (take) " << dbg(v) << std::endl);
                    g.take(w);
                    reduced = true;
                }
            }
        }
    }

    return reduced;
}

ReductionRule AlberSimpleRule2("AlberSimpleRule2 (dominated leaf removal)", alberSimpleRule2, 2, 1);

}  // namespace DSHunter