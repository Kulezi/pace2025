#include "../rrules.h"
namespace DSHunter {

bool singleDominatorRule(Instance& g) {
    auto nodes = g.nodes;
    bool reduced = false;
    for (auto v : nodes) {
        if (g.hasNode(v) && !g.isDominated(v) && g[v].dominators.size() == 1) {
            g.take(g[v].dominators.front());
            reduced = true;
        }
    }

    return reduced;
}

ReductionRule SingleDominatorRule("SingleDominatorRule", singleDominatorRule, 2, 1);

}  // namespace DSHunter
