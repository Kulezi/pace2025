#ifndef DS_VERIFIER_H
#define DS_VERIFIER_H
#include "../instance.h"
namespace DSHunter {

// Checks whether the given solution is a valid dominating set of the given instance.
// Throws a std::logic_error if it isn't.
void verify_solution(Instance g, const std::vector<int> solution) {
    for (auto u : g.nodes) g.setNodeStatus(u, UNDOMINATED);
    for (auto u : solution) {
        if (g.getNodeStatus(u) == TAKEN)
            throw std::logic_error("solution contains duplicates, one of which is vertex " +
                                   std::to_string(u));

        g.setNodeStatus(u, TAKEN);
        for (auto v : g.neighbourhoodExcluding(u)) g.setNodeStatus(v, DOMINATED);
    }

    for (auto u : g.nodes)
        if (g.getNodeStatus(u) == UNDOMINATED)
            throw std::logic_error("solution doesn't dominate vertex " + std::to_string(u));
}
}  // namespace DSHunter
#endif  // DS_VERIFIER_H