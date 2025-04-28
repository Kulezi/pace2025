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

    switch (cfg.solver_type) {
        case SolverType::Default: {
            if (g.forcedEdgeCount() == g.edgeCount()) {
                DS_TRACE(std::cerr << "running VC solver" << std::endl);
                VCSolver vs;
                g.ds = vs.solve(g);
                break;
            }

            TreewidthSolver ts(&cfg);
            DS_TRACE(std::cerr << "running treewidth solver" << std::endl);
            if (ts.solve(g)) {
                DS_TRACE(std::cerr << "treewidth solver success" << std::endl);
                break;
            }
            exit(1);
            DS_TRACE(std::cerr << "falling back to branching solver" << std::endl);
            BranchingSolver bs;
            std::vector<int> ds;
            bs.solve(g, ds);
            g.ds = ds;
            break;
        }

        case SolverType::TreewidthDP: {
            DS_TRACE(std::cerr << "running treewidth solver" << std::endl);
            TreewidthSolver ts(&cfg);
            if (!ts.solve(g))
                throw std::logic_error("treewidth dp failed (treewidth might be too big?)");
            break;
        }

        case SolverType::Bruteforce: {
            DS_TRACE(std::cerr << "running bruteforce solver" << std::endl);
            BruteforceSolver bs;
            bs.solve(g);
            break;
        }

        case SolverType::Branching: {
            DS_TRACE(std::cerr << "running branching solver" << std::endl);
            BranchingSolver bs;

            std::vector<int> ds;
            bs.solve(g, ds);
            g.ds = ds;
            break;
        }

        case SolverType::ReduceToVertexCover: {
            DS_TRACE(std::cerr << "running vertex cover solver" << std::endl);

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
            DS_TRACE(std::cerr << "running gurobi solver" << std::endl);

            GurobiSolver gs;
            if (!gs.solve(g)) {
                throw std::logic_error("gurobi didn't find a solution in time");
            }
            break;
        }

        default:
            throw std::logic_error("invalid solver_type value of " +
                                   std::to_string((int)cfg.solver_type) + " in SolverConfig");
    }

    sort(g.ds.begin(), g.ds.end());

    verify_solution(initial_instance, g.ds);
    return g.ds;
}

int presolve_complexity(PresolverType pt) {
    if (pt == PresolverType::Full)
        return 999;
    if (pt == PresolverType::Cheap)
        return 2;
    if (pt == PresolverType::None)
        return 0;
    throw std::logic_error("encountered incorrect PresolverType (" + std::to_string((int)pt) + ")");
}

void Solver::presolve(Instance &g) {
    reduce(g, cfg.reduction_rules, presolve_complexity(cfg.presolver_type));
}

}  // namespace DSHunter