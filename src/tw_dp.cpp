#include <iostream>
#include <fstream>
#include "ds.h"
#include "instance.h"
#include "rrules.h"
#include "td.h"
int main(int argc, char *argv[]) {
    std::ifstream f(argv[1]);
    Instance g(f);
    auto td = TreeDecomposition(g);
    DomSet::Exact ds(RRules::defaults_preprocess, RRules::defaults_branching);
    ds.solve_tw(g, std::cout);
}
