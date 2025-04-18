#include "rrules.h"
namespace DSHunter {
const std::vector<ReductionRule> get_default_reduction_rules() {
    static const std::vector<ReductionRule> rules = {
        DisregardRule,
        // Then rules that don't affect the graph structure.
        ForceEdgeRule,

        // First the cheap rules that only remove vertices.
        AlberSimpleRule1, AlberSimpleRule2, AlberSimpleRule3, AlberSimpleRule4,

        // Then more expensive rules.
        AlberMainRule1, AlberMainRule2};
    return rules;
}
}