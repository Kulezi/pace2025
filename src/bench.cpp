#include <iostream>

#include "ds.h"
#include "instance.h"
#include "rrules.h"
int main() {
    std::vector<RRules::Rule> reduction_rules = RRules::defaults;

    DomSet::Exact ds(reduction_rules);

    Instance g(std::cin);
    auto ans = ds.solve(g.clone(), std::cerr);
    
    std::cout << "branching_calls:" << ds.branch_calls << "\t" <<
                 "successful splits: " << ds.n_splits << std::endl;
    
    for (auto u : ans) {
        for (auto v : g.neighbourhood_including(u)) g.set_status(v, DOMINATED);
    }

    for (auto u : g.nodes()) assert(g.get_status(u) == DOMINATED);
}
