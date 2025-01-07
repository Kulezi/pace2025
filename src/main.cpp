#include <iostream>

#include "ds.h"
#include "instance.h"
#include "rrules.h"
int main() {
    std::vector<RRules::Rule> reduction_rules = RRules::defaults;

    DomSet::Exact ds(reduction_rules);

    Instance g(std::cin);
    auto ans = ds.solve(g.clone(), std::cout);

    std::cout << std::endl;

    for (auto u : ans) {
        // std::cerr << "u = " << u << " ";
        for (auto v : g.neighbourhood_including(u)) {
            // std::cerr << v << " ";
            g.set_status(v, DOMINATED);
        }
        // std::cerr << std::endl;
    }

    for (auto u : g.nodes()) assert(g.get_status(u) == DOMINATED);
}
