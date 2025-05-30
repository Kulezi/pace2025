#include "../rrules.h"
namespace DSHunter {

bool singleDominatorRule(Instance& g) {
    const auto nodes = g.nodes;
    bool reduced = false;
    for (const auto v : nodes) {
        if (g.hasNode(v) && !g.isDominated(v) && g[v].dominators.size() == 1) {
            g.take(g[v].dominators.front());
            reduced = true;
        }
    }

    return reduced;
}

ReductionRule SingleDominatorRule("SingleDominatorRule", singleDominatorRule, 2, 1);

}  // namespace DSHunter
