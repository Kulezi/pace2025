#include "solver.h"

#include "../rrules/rrules.h"
#include "branching/branching_solver.h"
#include "bruteforce/bruteforce_solver.h"
#include "gurobi/gurobi_solver.h"
#include "treewidth/treewidth_solver.h"
#include "vc/vc_solver.h"
#include "verifier.h"
namespace DSHunter {

std::vector<int> Solver::solve(Instance g) {
    auto initial_instance = g;
    presolve(g);

    if (g.nodes.empty()) {
        verify_solution(initial_instance, g.ds);
        return g.ds;
    }

    switch (config.solver_type) {
        case SolverType::Default:
        case SolverType::TreewidthDP: {
            TreewidthSolver ts;
            if (!ts.solve(g, config.decomposition_time_budget))
                throw std::logic_error("treewidth dp failed (treewidth might be too big?)");
            break;
        }

        case SolverType::Bruteforce: {
            BruteforceSolver bs;
            bs.solve(g);
            break;
        }

        case SolverType::Branching: {
            BranchingSolver bs;
            bs.solve(g, g.ds);
            break;
        }

        case SolverType::ReduceToVertexCover: {
            if (g.forcedEdgeCount() != g.edgeCount()) {
                throw std::logic_error(
                    "given instance contains unconstrained edges, making VC reduction "
                    "inapplicable");
            }

            VCSolver vs;
            g.ds = vs.solve(g);
            break;
        }

        case SolverType::Gurobi: {
            GurobiSolver gs;
            if (!gs.solve(g)) {
                throw std::logic_error("gurobi didn't find a solution in time");
            }
            break;
        }

        default:
            throw std::logic_error("invalid solver_type value of " +
                                   std::to_string((int)config.solver_type) + " in SolverConfig");
    }

    sort(g.ds.begin(), g.ds.end());
    verify_solution(initial_instance, g.ds);
    return g.ds;
}

int presolve_complexity(PresolverType pt) {
    if (pt == PresolverType::Full) return 999;
    if (pt == PresolverType::Cheap) return 2;
    if (pt == PresolverType::None) return 0;
    throw std::logic_error("encountered incorrect PresolverType (" + std::to_string((int)pt) + ")");
}

void Solver::presolve(Instance &g) {
    reduce(g, config.reduction_rules, presolve_complexity(config.presolver_type));
}

}  // namespace DSHunter