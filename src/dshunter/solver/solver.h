#ifndef DS_SOLVER_H
#define DS_SOLVER_H
#include <chrono>
#include <iostream>
#include <utility>

#include "../instance.h"
#include "../rrules/rrules.h"
namespace DSHunter {
using namespace std::chrono_literals;
enum class SolverType {
    Branching,
    TreewidthDP,
    Bruteforce,
    ReduceToVertexCover,
    Default,
};

enum class PresolverType {
    Full,
    Cheap,
    None
};

struct SolverConfig {
    std::vector<ReductionRule> reduction_rules;
    SolverType solver_type;
    PresolverType presolver_type;
    std::chrono::seconds decomposition_time_budget;
    std::string decomposer_path;
    int random_seed;
    int good_enough_treewidth;
    int max_treewidth;
    size_t max_memory_in_bytes;
    int max_bag_branch_depth;
    int max_branching_reductions_complexity;
    std::chrono::time_point<std::chrono::steady_clock> solve_start;

    SolverConfig(std::vector<ReductionRule> rrules, const SolverType st, const PresolverType pt)
        : reduction_rules(std::move(rrules)),
          solver_type(st),
          presolver_type(pt),
          decomposition_time_budget(300s),
          random_seed(0),
          good_enough_treewidth(14),
          max_treewidth(18),
          max_memory_in_bytes(14UL << 30UL),
          max_bag_branch_depth(7),
          max_branching_reductions_complexity(0) {}

    SolverConfig() : SolverConfig(DSHunter::get_default_reduction_rules(), SolverType::Default, PresolverType::Full) {}
    [[nodiscard]] int64_t millisElapsed() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - solve_start).count();
    }

    void logLine(const std::string &s) const {
        std::cerr << "c " << millisElapsed() << " " << s << std::endl; 
    }
};

struct Solver {
    SolverConfig cfg;
    Solver() : cfg(SolverConfig()) {}
    explicit Solver(SolverConfig sc) : cfg(std::move(sc)) {}

    std::vector<int> solve(Instance g);
    void presolve(Instance &g);

    private:
    std::vector<int> solveConnected(Instance &g);
};

}  // namespace DSHunter
#endif  // DS_SOLVER_H
