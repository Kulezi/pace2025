#include "../rrules.h"

namespace DSHunter {

bool forceEdgeRule(Instance& g) {
    auto nodes = g.nodes;
    bool reduced = false;
    for (auto v : nodes) {
        if (!std::binary_search(nodes.begin(), nodes.end(), v)) continue;
        if (g.deg(v) == 2 && !g.isDominated(v)) {
            auto e1 = g[v].adj[0];
            auto e2 = g[v].adj[1];
            
            if (g.isDisregarded(e1.to) && g.isDisregarded(e2.to))
            continue;
            
            if (g.hasEdge(e1.to, e2.to)) {
                if (e1.status == EdgeStatus::UNCONSTRAINED && e2.status == EdgeStatus::UNCONSTRAINED) {
                    DS_TRACE(std::cerr << __func__ << "(1)" << dbg(v) << std::endl);
                    g.removeNode(v);
                    if (g.getEdgeStatus(e1.to, e2.to) != EdgeStatus::FORCED)
                        g.forceEdge(e1.to, e2.to);
                    reduced = true;
                } else if (e1.status == EdgeStatus::FORCED && e2.status == EdgeStatus::UNCONSTRAINED && !g.isDisregarded(e1.to)) {
                    DS_TRACE(std::cerr << __func__ << "(2)" << dbg(v) << dbg(e1.to) << std::endl);
                    // Taking e1.to is optimal, as it's always better than taking v, and we are
                    // forced to take one of them.
                    g.take(e1.to);
                    reduced = true;
                } else if (e1.status == EdgeStatus::UNCONSTRAINED && e2.status == EdgeStatus::FORCED && !g.isDisregarded(e2.to)) {
                    DS_TRACE(std::cerr << __func__ << "(3)" << dbg(v) << dbg(e2.to) << std::endl);
                    // Taking e2.to is optimal, as it's always better than taking v, and we are
                    // forced to take one of them.
                    g.take(e2.to);
                    reduced = true;
                } else {
                    // We need to take either just u, or both e1.to and e2.to.
                    // TODO: contract them into a single vertex thats treated the following way:
                    // If it's taken into ds it adds e1.to and e2.to to the dominating set.
                    // If it's not taken it adds just v to the dominating set.
                }
            } else if (g.isDominated(e1.to) && g.isDominated(e2.to)) {
                g.removeNode(v);
                g.addEdge(e1.to, e2.to, EdgeStatus::FORCED);
                reduced = true;
            }
        }
    }

    return reduced;
}

ReductionRule ForceEdgeRule("ForceEdgeRule", forceEdgeRule, 1, 1);

}  // namespace DSHunter