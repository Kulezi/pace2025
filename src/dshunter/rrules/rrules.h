#ifndef RRULES_H
#define RRULES_H
#include <functional>

#include "../instance.h"
#include "../utils.h"

namespace DSHunter {

struct ReductionRule {
    std::string name;
    std::function<bool(Instance&)> f;

    // complexity = c if the worst case complexity of applying the rule is O(|G|^c).
    int complexity_dense, complexity_sparse;
    int application_count, success_count;
    ReductionRule(std::string name, std::function<bool(Instance&)> f, int complexity_dense, int complexity_sparse)
        : name(name),
          f(f),
          complexity_dense(complexity_dense),
          complexity_sparse(complexity_sparse),
          application_count(0),
          success_count(0) {
    }

    bool apply(Instance& g) const;
};

void reduce(Instance& g, std::vector<ReductionRule>& reduction_rules, int complexity = 999);

// Source: DOI 10.1007/s10479-006-0045-4, p. 4 (extended to handle forced edges)
// ~ O(|V|^3) for dense graphs, O(|V|) for sparse graphs.
bool alberMainRule1(Instance& g);

// Source: DOI 10.1007/s10479-006-0045-4, p. 4 (extended to handle forced edges)
// ~ O(|V|^4) for dense graphs, O(|V|^2) for sparse graphs.
bool alberMainRule2(Instance& g);

// Source: DOI 10.1007/s10479-006-0045-4, p. 6 (extended to handle forced edges)
// Applies the rule to as many applicable vertices as possible.
// ~ O(|G|^2) for dense graphs, O(|G|) for sparse graphs.
bool alberSimpleRule1(Instance& g);

// Source: DOI 10.1007/s10479-006-0045-4, p. 6 (extended to handle forced edges)
// ~ O(|G|^2) for dense graphs, O(|G|) for sparse graphs.
bool alberSimpleRule2(Instance& g);

// Source: DOI 10.1007/s10479-006-0045-4, p. 6 (extended to handle forced edges)
// ~ O(|G|^2) for dense graphs, O(|G|) for sparse graphs.
bool alberSimpleRule3(Instance& g);

// Source: DOI 10.1007/s10479-006-0045-4, p. 6  (extended to handle forced edges)
// ~ O(|G|^2) for dense graphs. O(|G|) for sparse graphs.
bool alberSimpleRule4(Instance& g);

// If a vertex of degree two is contained in the neighbourhoods of both its neighbours,
// and they are connected by an edge, make the edge forced and remove this vertex, as there
// exists an optimal solution not-taking this vertex and taking one of its neighbours.
// ~ O(|G|) for any graph.
bool forceEdgeRule(Instance& g);

bool disregardRule(Instance& g);

bool removeDisregardedRule(Instance& g);

bool disregardedNeighbourhoodRule(Instance& g);

extern ReductionRule AlberMainRule1;
extern ReductionRule AlberMainRule2;
extern ReductionRule AlberSimpleRule1;
extern ReductionRule AlberSimpleRule2;
extern ReductionRule AlberSimpleRule3;
extern ReductionRule AlberSimpleRule4;

extern ReductionRule ForceEdgeRule;

extern ReductionRule DisregardRule;
extern ReductionRule RemoveDisregardedRule;
extern ReductionRule DisregardedNeighbourhoodRule;

const std::vector<ReductionRule> get_default_reduction_rules();
}  // namespace DSHunter
#endif  // RRULES_H
