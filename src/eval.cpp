#include <fstream>
#include <iostream>
#include "bounds.h"
#include "ds.h"
#include "instance.h"
#include "nice_tree_decomposition.h"
#include "rrules.h"
using namespace std;

template <typename T>
void log_column(string name, string suffix, T value) {
    std::cerr << "," << name + (suffix == "" ? "" : "_") + suffix;
    std::cout << fixed <<  "," << value;
}


// Prints the csv header to cerr.
int main(int argc, char *argv[]) {
    std::cout.precision(3);
    // n_init, m_init, lb_init, ub_init, tw_init, n_cheap, m_cheap, lb_cheap, ub_cheap, tw_cheap,
    // ruletimes..., n_exp, m_exp, lb_exp, ub_exp, tw_exp, ruletimes...
    DSHunter::Instance g_init(std::cin);
    DSHunter::Exact ds_init(DSHunter::RRules::defaults_preprocess, DSHunter::RRules::defaults_branching);

    DSHunter::Instance g_cheap(g_init);
    DSHunter::Exact ds_cheap(DSHunter::RRules::defaults_preprocess, DSHunter::RRules::defaults_branching);
    ds_cheap.reduce_branch(g_cheap);

    DSHunter::Instance g_exp(g_cheap);
    DSHunter::Exact ds_exp(DSHunter::RRules::defaults_preprocess, DSHunter::RRules::defaults_branching);
    ds_exp.reduce(g_exp);

    vector<pair<string, DSHunter::Instance>> instances = {
        {"init", g_init}, {"cheap", g_cheap}, {"exp", g_exp}};

    for (auto &[suffix, instance] : instances) log_column("n", suffix, instance.nodeCount());
    for (auto &[suffix, instance] : instances) log_column("m", suffix, instance.edgeCount());
    for (auto &[suffix, instance] : instances)
        log_column("m_forced", suffix, instance.forcedEdgeCount());
    for (auto &[suffix, instance] : instances)
        log_column("lb", suffix, DSHunter::lower_bound(instance));
    for (auto &[suffix, instance] : instances)
        log_column("ub", suffix, DSHunter::upper_bound(instance));

    int cnt = 0;
    for (auto i : ds_cheap.benchmark_info.rule_branch_time) {
        log_column("rule" + to_string(cnt), "cheap", i.count());
    }

    for (auto i : ds_exp.benchmark_info.rule_time) {
        log_column("rule" + to_string(cnt), "exp", i.count());
    }

    vector<pair<string, DSHunter::NiceTreeDecomposition>> decompositions;
    for (auto &[suffix, instance] : instances) {
        decompositions.push_back(
            {suffix, DSHunter::NiceTreeDecomposition(instance, DSHunter::GOOD_ENOUGH_TREEWIDTH)});
    }

    for (auto &[suffix, decomposition] : decompositions)
        log_column("tw", suffix, decomposition.width());

    for (auto &[suffix, decomposition] : decompositions)
        log_column("td_size", suffix, decomposition.n_nodes());

    log_column("decomp_time", "init", ds_init.benchmark_info.treewidth_decomposition_time.count());
    log_column("decomp_time", "cheap",
               ds_cheap.benchmark_info.treewidth_decomposition_time.count());
    log_column("decomp_time", "exp", ds_exp.benchmark_info.treewidth_decomposition_time.count());

    std::ofstream sol((std::string(argv[1]) + ".sol"));

    auto ans = ds_init.solve(g_init, sol);
    log_column("ds_size", "", ans.size());
    log_column("tw_dp_time", "", ds_init.benchmark_info.treewidth_calculation_time.count());
    std::cerr << std::endl;
    std::cout << std::endl;
}
