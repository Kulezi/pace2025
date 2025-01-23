#include <iostream>

#include "ds.h"
#include "instance.h"
#include "rrules.h"
int main() {
    DomSet::Exact ds(RRules::defaults_preprocess, RRules::defaults_preprocess);

    Instance g(std::cin);
    auto ans = ds.solve(g, std::cerr);
    
    std::cout << "branching_calls:" << ds.branch_calls << "\t" <<
                 "successful splits: " << ds.n_splits << std::endl;
    
    for (auto u : ans) {
        for (auto v : g.neighbourhoodIncluding(u)) g.setStatus(v, DOMINATED);
    }

    for (auto u : g.nodes) assert(g.getStatus(u) == DOMINATED);
}
