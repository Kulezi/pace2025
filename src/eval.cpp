#include <fstream>
#include <iostream>

#include "dshunter/dshunter.h"
using namespace std;

template <typename T>
void log_column(string name, string suffix, T value) {
    std::cerr << "," << name + (suffix == "" ? "" : "_") + suffix;
    std::cout << fixed << "," << value;
}

constexpr int COMPLEXITY_CHEAP = 3;

// Prints the csv header to cerr.
int main(int argc, char *argv[]) {
    auto rules = DSHunter::default_reduction_rules;
    std::cout.precision(3);

    DSHunter::Instance g_init(std::cin);
    DSHunter::Instance g_cheap(g_init);
    reduce(g_cheap, rules, COMPLEXITY_CHEAP);

    DSHunter::Instance g_exp(g_cheap);
    reduce(g_exp, rules);

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

    vector<pair<string, DSHunter::NiceTreeDecomposition>> decompositions;

    for (auto &[suffix, instance] : instances) {
        decompositions.push_back(
            {suffix, DSHunter::NiceTreeDecomposition(instance, DSHunter::GOOD_ENOUGH_TREEWIDTH)});
    }

    for (auto &[suffix, decomposition] : decompositions)
        log_column("tw", suffix, decomposition.width());

    for (auto &[suffix, decomposition] : decompositions)
        log_column("td_size", suffix, decomposition.n_nodes());

    std::ofstream sol((std::string(argv[1]) + ".sol"));

    auto ans = DSHunter::Solver().solve(g_init);
    log_column("ds_size", "", ans.size());
    
    std::cerr << std::endl;
    std::cout << std::endl;
}
