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

        int n_old = g.nodeCount(),
            m_old = g.edgeCount(),
            d_old = g.disregardedNodeCount(),
            f_old = g.forcedEdgeCount();

        int u_old = 0;
        for (auto u : g.nodes) {
            if (g.isDominated(u)) u_old++;
        }


        bool reduced = rule.apply(g);
        ++rule.application_count;

        if (reduced) {
            ++rule.success_count;
            rule.delta_f += f_old - g.forcedEdgeCount();
            int u_new = 0;
            for (auto u : g.nodes) if (g.isDominated(u)) u_new++;
            rule.delta_u += u_old - u_new;
            goto _start;
        }
    }
}

}  // namespace DSHunter
