#include "rrules.h"
namespace DSHunter {
bool ReductionRule::apply(Instance& g) const {
    // DS_TRACE(std::cerr << std::format("trying to apply {} (n={}, m={}, f={}, d={})", name, g.nodeCount(), g.edgeCount(), g.forcedEdgeCount(), g.disregardedNodeCount()) << std::endl);

    bool applied = f(g);

    // DS_TRACE(if (applied) std::cerr << std::format("successfully applied {} (n={}, m={}, f={}, d={})", name, g.nodeCount(), g.edgeCount(), g.forcedEdgeCount(), g.disregardedNodeCount()) << std::endl;
    //          else std::cerr << "failed to apply " << name << std::endl;);
    return applied;
}

void reduce(Instance& g, std::vector<ReductionRule>& reduction_rules, int complexity) {
_start:
    for (auto& rule : reduction_rules) {
        if (rule.complexity_dense > complexity)
            continue;
        bool reduced = rule.apply(g);
        ++rule.application_count;

        if (reduced) {
            ++rule.success_count;
            goto _start;
        }
    }
}

}  // namespace DSHunter
