#ifndef DS_H
#define DS_H
#include "instance.h"
#include "nice_tree_decomposition.h"
#include "rrules.h"
#include "ternary.h"
#include "utils.h"
#ifdef DS_BENCHMARK
#include <chrono>
#endif
namespace DSHunter {

constexpr int UNSET = -1, INF = 1'000'000;

#ifdef DS_BENCHMARK

struct BenchmarkInfo {
    size_t branch_calls = 0;
    size_t n_splits = 0;
    size_t max_encountered_treewidth = 0;
    using millis = std::chrono::duration<double, std::milli>;
    std::vector<millis> rule_time;
    std::vector<millis> rule_branch_time;
    millis treewidth_decomposition_time;
    millis treewidth_calculation_time;
    millis treewidth_joins_time;

    BenchmarkInfo() = default;
    BenchmarkInfo(std::vector<RRules::Rule> rules, std::vector<RRules::Rule> rules_branch)
        : rule_time(rules.size(), millis(0)),
          rule_branch_time(rules_branch.size(), millis(0)),
          treewidth_decomposition_time(0),
          treewidth_calculation_time(0),
          treewidth_joins_time(0) {}
};
#endif

constexpr uint64_t MAX_MEMORY_IN_BYTES = (1UL << 30);
constexpr size_t MAX_HANDLED_TREEWIDTH = 18;
constexpr size_t GOOD_ENOUGH_TREEWIDTH = 15;

// Checks whether the given solution is a valid dominating set of the given instance.
// Throws a std::logic_error if it isn't.
void verify_solution(Instance g, const std::vector<int> solution);

struct Exact {
    std::vector<RRules::Rule> rules, rules_branch;
    std::vector<std::vector<int>> c;
    #ifdef DS_BENCHMARK
    BenchmarkInfo benchmark_info;
    #endif  // DS_BENCHMARK
    Exact(std::vector<RRules::Rule> _rules, std::vector<RRules::Rule> _rules_branch)
        : rules(_rules), rules_branch(_rules_branch) {
#ifdef DS_BENCHMARK
        benchmark_info = BenchmarkInfo(rules, rules_branch);
#endif
    }

    std::vector<int> solve(Instance g, std::ostream &out);

    // Returns true if instance was solved,
    // false if the width of found decompositions was too big to handle.
    bool solveTreewidth(Instance &g);

    void solveBranching(const Instance g, std::vector<int> &best_ds, int level = 0);

    std::vector<int> solveBruteforce(Instance g, std::ostream &out);

    void reduce(Instance &g);

    void reduce_branch(Instance &g);

    void print(std::vector<int> ds, std::ostream &out);

   private:
    void take(Instance g, int v, std::vector<int> &best_ds, int level);

    inline int cost(const Instance &g, int v);

    // [Parameterized Algorithms [7.3.2] - 10.1007/978-3-319-21275-3] extended to handle forced
    // edges.
    int getC(const Instance &g, NiceTreeDecomposition &td, int t, TernaryFun f);

    void recoverDS(Instance &g, NiceTreeDecomposition &td, int t, TernaryFun f);

    uint64_t getMemoryUsage(const NiceTreeDecomposition &td);
};
}  // namespace DSHunter
#endif  // DS_H