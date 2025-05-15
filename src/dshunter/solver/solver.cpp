#include "solver.h"

#include "../bounds.h"
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
    cfg.logLine(std::format("{} <= |D| <= {}", lowerBound(g), upperBound(g)));

    if (g.nodes.empty()) {
        verify_solution(initial_instance, g.ds);
        return g.ds;
    }

    std::vector<int> ds = g.ds;
    auto components = g.split();

    cfg.logLine(std::format("reduced graph has {} components", components.size()));
    for (size_t i = 0; i < components.size(); i++) {
        g.ds.clear();
        g.nodes = components[i];
        cfg.logLine(std::format("solving component {}/{} with n={}, m={}", i+1, components.size(), g.nodeCount(), g.edgeCount()));
        auto component_ds = solveConnected(g);
        cfg.logLine(std::format("solved component {}/{} with ds of size {} out of n={} nodes", i+1, components.size(), component_ds.size(), component_ds.size(), g.nodeCount()));
        ds.insert(ds.begin(), component_ds.begin(), component_ds.end());
    }

    std::ranges::sort(ds);

    cfg.logLine("verifying solution");
    verify_solution(initial_instance, ds);
    cfg.logLine(std::format("solution of size {} verified", ds.size()));
    return ds;
}

std::vector<int> Solver::solveConnected(Instance &g) {
    switch (cfg.solver_type) {
        case SolverType::Default: {
            if (g.forcedEdgeCount() == g.edgeCount()) {
                cfg.logLine("running vc solver");
                VCSolver vs;
                return vs.solve(g);
            }

            TreewidthSolver ts(&cfg);
            cfg.logLine("running treewidth solver");
            if (ts.solve(g)) {
                cfg.logLine("treewidth solver success");
                return g.ds;
            }

            cfg.logLine("treewidth solver failed, falling back to branching solver");
            return BranchingSolver(&cfg).solve(g);
        }

        case SolverType::TreewidthDP: {
            cfg.logLine("running treewidth solver");
            TreewidthSolver ts(&cfg);
            if (!ts.solve(g))
                throw std::logic_error("treewidth dp failed (treewidth might be too big?)");
            return g.ds;
        }

        case SolverType::Bruteforce: {
            cfg.logLine("running bruteforce solver");
            BruteforceSolver bs;
            bs.solve(g);
            return g.ds;
        }

        case SolverType::Branching: {
            cfg.logLine("running branching solver");
            return BranchingSolver(&cfg).solve(g);
        }

        case SolverType::ReduceToVertexCover: {
            cfg.logLine("running vc solver");

            if (g.forcedEdgeCount() != g.edgeCount()) {
                throw std::logic_error(
                    "given instance contains unconstrained edges, making VC reduction "
                    "inapplicable");
            }

            VCSolver vs;
            return vs.solve(g);
        }

        case SolverType::Gurobi: {
            cfg.logLine("running gurobi solver");

            auto sol = GurobiSolver(g).solve();
            if (!sol.has_value()) {
                throw std::logic_error("gurobi didn't find a solution in time");
            }
            return sol.value();
        }

        default:
            throw std::logic_error(std::format("invalid solver_type value of {} in SolverConfig", static_cast<int>(cfg.solver_type)));
    }
}

int presolve_complexity(PresolverType pt) {
    if (pt == PresolverType::Full)
        return 999;
    if (pt == PresolverType::Cheap)
        return 2;
    if (pt == PresolverType::None)
        return 0;
    throw std::logic_error(std::format("encountered incorrect PresolverType ({})", static_cast<int>(pt)));
}

void Solver::presolve(Instance &g) {
    reduce(g, cfg.reduction_rules, presolve_complexity(cfg.presolver_type));
}

}  // namespace DSHunter