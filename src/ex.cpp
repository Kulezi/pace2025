#include <iostream>

#include "bounds.h"
#include "ds.h"
#include "instance.h"
#include "rrules.h"
int main() {
    DomSet::Exact ds(RRules::defaults_preprocess, RRules::defaults_preprocess);

    Instance g(std::cin);
    std::map<int, int> cnt;

    for (auto u : g.nodes) {
        cnt[(int)g.adj[u].size()]++;
    }

    for (auto [k, v] : cnt) {
        std::cerr << "deg(" << k << ") = " <<  v << std::endl;
    }

    
}
