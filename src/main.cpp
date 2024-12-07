#include <iostream>

#include "ds.h"
#include "instance.h"
#include "rrules.h"
int main() {
    std::vector<RRules::Rule> reduction_rules = {
        RRules::AlberMainRule1,
        RRules::AlberMainRule2,
        // RRules::AlberSimpleRule1,
        // RRules::AlberSimpleRule2,
        // RRules::AlberSimpleRule3,
        // RRules::AlberSimpleRule4,
    };

    DomSet::Exact ds(reduction_rules);


    Instance g(std::cin);
    ds.solve(g, std::cout);
    std::cout << std::endl;
    
    auto ans = ds.best_ds;
    for (auto u : ans) {
        for (auto v : g.neighbourhood_including(u)) g.set_status(v, DOMINATED);
    }

    for (auto u : g.nodes) assert(g.get_status(u) == DOMINATED);
}
