#ifndef RRULES_H
#define RRULES_H
#include <functional>

#include "instance.h"

namespace DSHunter {

namespace RRules {

using Rule = std::function<bool(Instance&)>;

// Naive implementation of Main Rule 1 - DOI 10.1007/s10479-006-0045-4, p. 4
// ~ O(|V|^2) or O(|V|^3) depending on the remove_node operation complexity.
bool AlberMainRule1(Instance& g);

// Naive implementation of Main Rule 2 - DOI 10.1007/s10479-006-0045-4, p. 4
// ~ O(|V|^2) or O(|V|^3) depending on the remove_node operation complexity.
bool AlberMainRule2(Instance& g);

// Batched implementation of Simple Rule 1 - DOI 10.1007/s10479-006-0045-4, p. 6
// ~ O(|E| + |V| * (# removed edges)) depending on the remove_node operation complexity.
bool AlberSimpleRule1(Instance& g);

// Naive implementation of Simple Rule 2 - DOI 10.1007/s10479-006-0045-4, p. 6
// ~ O(|V| * (# removed nodes)) depending on the remove_node operation complexity.
bool AlberSimpleRule2(Instance& g);
// Naive implementation of Simple Rule 3 - DOI 10.1007/s10479-006-0045-4, p. 6
// ~ O(|V|^2 * (# removed nodes)) depending on the remove_node operation complexity.
bool AlberSimpleRule3(Instance& g);

// Naive implementation of Simple Rule 4 - DOI 10.1007/s10479-006-0045-4, p. 6
// ~ O(|V| * (# removed nodes)) depending on the remove_node operation complexity.
bool AlberSimpleRule4(Instance& g);

// If a vertex of degree two is contained in the neighbourhoods of both its neighbours,
// and they are connected by an edge, make the edge forced and remove this vertex, as there
// exists an optimal solution not-taking this vertex and taking one of its neighbours.
// Complexity: ~ O(|V| * (# removed nodes)) depending on the remove_node operation complexity.
bool ForcedEdgeRule(Instance& g);

const std::vector<RRules::Rule> defaults_preprocess = {
    ForcedEdgeRule,   AlberSimpleRule1, AlberSimpleRule2, AlberSimpleRule3,
    AlberSimpleRule4, AlberMainRule1,   AlberMainRule2,
};

const std::vector<RRules::Rule> defaults_branching = {
    ForcedEdgeRule,   AlberSimpleRule1, AlberSimpleRule2,
    AlberSimpleRule3, AlberSimpleRule4, AlberMainRule1,
};

}  // namespace RRules
}  // namespace DSHunter
#endif  // RRULES_H
