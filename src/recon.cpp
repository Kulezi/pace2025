#include <iostream>

#include "bounds.h"
#include "ds.h"
#include "instance.h"
#include "rrules.h"
#include "td.h"

void print_tw(Instance &g, std::string suffix) {
    std::cerr << ",tw" + suffix;
    std::cout << "," << TreeDecomposition(g).width();
}

void print_info(Instance &g, std::string suffix) {
    std::cerr << ",n" + suffix + ",m" + suffix + ",lb" + suffix + ",ub" + suffix;
    std::cout << "," << g.nodeCount() << "," << g.edgeCount() << "," << bounds::lower_bound(g)
              << "," << bounds::upper_bound(g);
    print_tw(g, suffix);
}

// Prints the csv header to cerr.
int main() {
    DomSet::Exact ds(RRules::defaults_preprocess, RRules::defaults_branching);
    // n_init, m_init, lb_init, ub_init, tw_init, n_cheap, m_cheap, lb_cheap, ub_cheap, tw_cheap,
    // ruletimes..., n_exp, m_exp, lb_exp, ub_exp, tw_exp, ruletimes...
    Instance g(std::cin);
    print_info(g, "_init");

    ds.reduce_branch(g);
    print_info(g, "_cheap");

    auto &info = ds.benchmark_info;
    for (size_t i = 0; i < info.rule_branch_time.size(); i++) {
        std::cerr << ",rule_cheap_" << i;
        std::cout << "," << info.rule_branch_time[i].count();
    }

    ds.reduce(g);
    print_info(g, "_exp");
    for (size_t i = 0; i < info.rule_time.size(); i++) {
        std::cerr << ",rule_exp_" << i;
        std::cout << "," << info.rule_time[i].count();
    }
    std::cerr << "\n";
}
