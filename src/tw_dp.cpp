#include <iostream>

#include "ds.h"
#include "instance.h"
#include "rrules.h"
int main() {
    DomSet::Exact ds(RRules::defaults_preprocess, RRules::defaults_preprocess);

    Instance g(std::cin);
    auto ans = ds.solve_tw(g, std::cout);

    std::cout << std::endl;

    for (auto u : ans) {
        for (auto v : g.neighbourhood_including(u)) g.set_status(v, DOMINATED);
    }

    for (auto u : g.nodes) assert(g.get_status(u) == DOMINATED);
}
