#include <iostream>

#include "ds.h"
#include "graph.h"
#include "rrules.h"
int main() {
    std::vector<RRules::Rule> reduction_rules = {
        // RRules::AlberMainRule1,
        // RRules::AlberMainRule2,
        // RRules::AlberSimpleRule1,
        // RRules::AlberSimpleRule2,
    };

    DomSet::Exact ds(Graph(std::cin), reduction_rules);

    ds.solve_bruteforce(std::cout);
}
