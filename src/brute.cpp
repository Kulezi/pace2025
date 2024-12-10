#include <iostream>

#include "ds.h"
#include "instance.h"
#include "rrules.h"
int main() {
    std::vector<RRules::Rule> reduction_rules = RRules::defaults;

    DomSet::Exact ds(reduction_rules);

    ds.solve_bruteforce(Instance(std::cin), std::cout);
}
