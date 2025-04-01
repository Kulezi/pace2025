#include <iostream>

#include "ds.h"
#include "instance.h"
#include "rrules.h"
int main() {
    DSHunter::Exact ds({}, {});

    DSHunter::Instance g(std::cin);
    auto sol = ds.solveBruteforce(g, std::cout);
}
