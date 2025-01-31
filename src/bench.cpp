#include <iostream>

#include "ds.h"
#include "instance.h"
#include "rrules.h"
#include "bounds.h"
int main() {
    DomSet::Exact ds(RRules::defaults_preprocess, RRules::defaults_preprocess);

    Instance g(std::cin);
    std::cout << "nodes: " << g.nodeCount() << "\n";
    std::cout << "edges: " << g.edgeCount() << "\n";
    std::cout << "initial upper_bound: " << bounds::upper_bound(g) << "\n";
    std::cout << "initial lower_bound: " << bounds::lower_bound(g) << "\n";

    auto start = std::chrono::high_resolution_clock::now();
    auto ans = ds.solve(g, std::cerr);
    auto time = std::chrono::high_resolution_clock::now() - start;
    std::cout << "time: " << std::chrono::duration<double, std::milli>(time).count() << "ms\n";
    std::cout << "dominating set size: " << ans.size() << "\n";
    for (auto u : ans) {
        for (auto v : g.neighbourhoodIncluding(u)) g.setStatus(v, DOMINATED);
    }

    for (auto u : g.nodes) assert(g.getStatus(u) == DOMINATED);

    auto &info = ds.benchmark_info;
    std::cout << "branching_calls:" << info.branch_calls << "\n"
              << "successful splits: " << info.n_splits << "\n"
              << "max_encountered_treewidth: " << info.max_encountered_treewidth << "\n"
              << "tree_decomposition_time: " << info.treewidth_decomposition_time.count() << "ms\n"
              << "treewidth_dp_time: " << info.treewidth_calculation_time.count() << "ms\n"
              << "treewidth_joins_time: " << info.treewidth_joins_time.count() << "ms\n";

    auto c = ds.c;
    size_t total_dp_values = 0, unset_values = 0, inf_values = 0;
    for (auto v : c) {
        total_dp_values += v.size();
        for (auto u : v) {
            if (u >= DomSet::INF) inf_values++;
            if (u == DomSet::UNSET) unset_values++;
        }
    }

    std::cout << "total_dp_values: " << total_dp_values << "\n"
              << "unset_values: " << unset_values << "\n"
              << "inf_values: " << inf_values << "\n";

    std::cout << "time spent reducing per reduction:\n";
    for (size_t i = 0; i < info.rule_time.size(); i++)
        std::cout << "RRules::defaults[" << i << "]:" << info.rule_time[i].count() << "ms\n";
    std::cout << "\n";

    std::cout << "time spent reducing per branching reduction:\n";
    for (size_t i = 0; i < info.rule_branch_time.size(); i++)
        std::cout << "RRules::defaults_branching[" << i << "]:" << info.rule_branch_time[i].count()
                  << "ms\n";
    std::cout << "\n";
}
