#include "rrules.h"
namespace DSHunter {
const std::vector<ReductionRule> get_default_reduction_rules() {
    static const std::vector<ReductionRule> rules = {
        ForceEdgeRule,

        // Rules regarding disregarding.
        DisregardRule,
        DominatedNeighbourhoodMarkingRule,
        RemoveDisregardedRule,
        SingleDominatorRule,
        SameDominatorsRule,

        // Cheap rules that only remove vertices.
        AlberSimpleRule1,
        AlberSimpleRule2,
        AlberSimpleRule3,
        AlberSimpleRule4,

        // Then more expensive rules.
        AlberMainRule1,
        AlberMainRule2
    };
    return rules;
}
}  // namespace DSHunter