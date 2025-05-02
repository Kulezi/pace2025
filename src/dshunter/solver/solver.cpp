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
    cfg.solve_start = std::chrono::steady_clock::now();
    auto initial_instance = g;

    int n_old = g.nodeCount();
    int m_old = g.edgeCount();
    cfg.logLine("starting presolve");
    presolve(g);
    cfg.logLine("presolve done");
    cfg.logLine(std::format("reduced n from {} to {}", n_old, g.nodeCount()));
    cfg.logLine(std::format("disregarded node count {}", ([&]() { int res = 0; for (auto v : g.nodes) if (g.isDisregarded(v)) res++; return res; })()));
    cfg.logLine(std::format("reduced m from {} to {}", m_old, g.edgeCount()));
    cfg.logLine(std::format("forced edge count {}", g.forcedEdgeCount()));

    if (g.nodes.empty()) {
        verify_solution(initial_instance, g.ds);
        return g.ds;
    }

    switch (cfg.solver_type) {
        case SolverType::Default: {
            if (g.forcedEdgeCount() == g.edgeCount()) {
                cfg.logLine("running vc solver");
                VCSolver vs;
                g.ds = vs.solve(g);
                break;
            }

            TreewidthSolver ts(&cfg);
            cfg.logLine("running treewidth solver");
            if (ts.solve(g)) {
                cfg.logLine("treewidth solver success");
                break;
            }

            cfg.logLine("treewidth solver failed, falling back to branching solver");
            BranchingSolver bs;
            std::vector<int> ds;
            bs.solve(g, ds);
            g.ds = ds;
            break;
        }

        case SolverType::TreewidthDP: {
            cfg.logLine("running treewidth solver");
            TreewidthSolver ts(&cfg);
            if (!ts.solve(g))
                throw std::logic_error("treewidth dp failed (treewidth might be too big?)");
            break;
        }

        case SolverType::Bruteforce: {
            cfg.logLine("running bruteforce solver");
            BruteforceSolver bs;
            bs.solve(g);
            break;
        }

        case SolverType::Branching: {
            cfg.logLine("running branching solver");
            BranchingSolver bs;

            std::vector<int> ds;
            bs.solve(g, ds);
            g.ds = ds;
            break;
        }

        case SolverType::ReduceToVertexCover: {
            cfg.logLine("running vc solver");

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
            cfg.logLine("running gurobi solver");

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

    cfg.logLine("verifying solution");
    verify_solution(initial_instance, g.ds);
    cfg.logLine("solution of size " + std::to_string(g.ds.size()) + "verified");
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