#include <iostream>

#include "ds.h"
#include "instance.h"
#include "rrules.h"
int main() {
    DomSet::Exact ds(RRules::defaults_preprocess, RRules::defaults_preprocess);

    Instance g(std::cin);

    auto start = std::chrono::high_resolution_clock::now();
    auto ans = ds.solve(g, std::cerr);
    auto time = std::chrono::high_resolution_clock::now() - start;
    std::cout << "time: " << std::chrono::duration<double, std::milli>(time).count() << "ms\n";

    for (auto u : ans) {
        for (auto v : g.neighbourhoodIncluding(u)) g.setStatus(v, DOMINATED);
    }

    for (auto u : g.nodes) assert(g.getStatus(u) == DOMINATED);

    auto &info = ds.benchmark_info;
    std::cout << "branching_calls:" << info.branch_calls << "\n"
              << "successful splits: " << info.n_splits << "\n"
              << "max_encountered_treewidth: " << info.max_encountered_treewidth << "\n"
              << "tree_decomposition_time: " << info.treewidth_decompositon_time.count() << "\n";

    std::cout << "time spent reducing per reduction:\n";
    for (size_t i = 0; i < info.rule_time.size(); i++) std::cout << "RRules::defaults[" << i << "]:" << info.rule_time[i].count() << "ms\n";
    std::cout << "\n";
    std::cout << "time spent reducing per reduction:\n";
    for (size_t i = 0; i < info.rule_branch_time.size(); i++) std::cout << "RRules::defaults_branching[" << i << "]:" << info.rule_branch_time[i].count() << "ms\n";
    std::cout << "\n";
}
