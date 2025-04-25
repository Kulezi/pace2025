#ifndef DS_SOLVER_H
#define DS_SOLVER_H
#include <chrono>

#include "../instance.h"
#include "../rrules/rrules.h"
#include "../utils.h"
namespace DSHunter {
using namespace std::chrono_literals;
enum class SolverType {
    Branching,
    TreewidthDP,
    Bruteforce,
    ReduceToVertexCover,
    Default,
    Gurobi
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

    SolverConfig(std::vector<ReductionRule> rrules, SolverType st, PresolverType pt)
        : reduction_rules(rrules), solver_type(st), presolver_type(pt) {
    }

    SolverConfig()
        : reduction_rules(DSHunter::get_default_reduction_rules()),
          solver_type(SolverType::Default),
          presolver_type(PresolverType::Full),
          decomposition_time_budget(10s),
          decomposer_path() {
    }
};

struct Solver {
    SolverConfig config;
    Solver() : config(SolverConfig()) {
    }
    Solver(SolverConfig sc) : config(sc) {
    }

    std::vector<int> solve(Instance g);
    void presolve(Instance &g);
};

}  // namespace DSHunter
#endif  // DS_SOLVER_H