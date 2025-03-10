#include <iostream>

#include "ds.h"
#include "instance.h"
#include "rrules.h"
int main() {
    DomSet::Exact ds({}, {});

    Instance g(std::cin);
    auto sol = ds.solveBruteforce(g, std::cout);
}
