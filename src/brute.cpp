#include <iostream>

#include "ds.h"
#include "instance.h"
#include "rrules.h"
int main() {
    DomSet::Exact ds(RRules::defaults_preprocess, RRules::defaults_preprocess);

    Instance g(std::cin);
    auto sol = ds.solveBruteforce(g, std::cout);
}
