#include "gurobi_solver.h"

#include <memory>

#include "../../utils.h"
#include "gurobi_c++.h"
namespace {

std::vector<int> getRV(const DSHunter::Instance& g) {
    auto rv = std::vector<int>(g.all_nodes.size());
    for (size_t i = 0; i < g.nodes.size(); i++) {
        int v = g.nodes[i];
        rv[v] = i;
    }

    return rv;
}

}  // namespace

namespace DSHunter {
GurobiSolver::GurobiSolver(const Instance& g) : g(g), rv(getRV(g)), model(getEnv()) {
    try {
        is_selected = std::vector<GRBVar>(g.nodes.size());
        for (size_t i = 0; i < g.nodes.size(); ++i) {
            is_selected[i] =
                model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "is_selected_" + std::to_string(g.nodes[i]));
        }

        for (int v : g.nodes) {
            GRBLinExpr node_constraint = 0;
            for (auto [u, status] : g[v].adj) {
                node_constraint += is_selected[rv[u]];

                if (status == EdgeStatus::FORCED) {
                    GRBLinExpr edge_constraint = 0;
                    edge_constraint += is_selected[rv[u]];
                    edge_constraint += is_selected[rv[v]];
                    model.addConstr(edge_constraint >= 1);
                }
            }

            node_constraint += is_selected[rv[v]];
            if (!g.isDominated(v)) {
                model.addConstr(node_constraint >= 1);
            }

            if (g.isDisregarded(v)) {
                GRBLinExpr disregard_constraint = 0;
                disregard_constraint += is_selected[rv[v]];
                model.addConstr(disregard_constraint <= 0);
            }
        }

        GRBLinExpr obj = 0;
        for (int v : g.nodes) {
            obj += is_selected[rv[v]];
        }

        model.setObjective(obj, GRB_MINIMIZE);
    } catch (GRBException& e) {
        std::cerr << "c gurobi init error code = " << e.getErrorCode() << std::endl;
        std::cerr << "c gurobi init error message: " << e.getMessage() << std::endl;
        throw std::logic_error("gurobi init failed");
    }
}

std::optional<std::vector<int>> GurobiSolver::solve() {
    try {
        model.optimize();
        if (model.get(GRB_IntAttr_Status) != GRB_OPTIMAL)
            return std::nullopt;

        std::vector<int> ds = g.ds;
        for (size_t i = 0; i < g.nodes.size(); ++i) {
            if (is_selected[i].get(GRB_DoubleAttr_X) > 0)
                ds.push_back(g.nodes[i]);
        }

        return ds;
        ;
    } catch (GRBException e) {
        std::cerr << "c gurobi error code = " << e.getErrorCode() << std::endl;
        std::cerr << "c gurobi error message: " << e.getMessage() << std::endl;
    } catch (...) {
        std::cerr << "c gurobi exception during optimization" << std::endl;
    }

    return std::nullopt;
}

int GurobiSolver::lowerBound() {
    try {
        model.optimize();
        return static_cast<int>(model.get(GRB_DoubleAttr_ObjBound));
    } catch (GRBException e) {
        std::cerr << "c gurobi error code = " << e.getErrorCode() << std::endl;
        std::cerr << "c gurobi error message: " << e.getMessage() << std::endl;
    } catch (...) {
        std::cerr << "c gurobi exception during optimization" << std::endl;
    }

    return 0;
}

GRBEnv& GurobiSolver::getEnv() {
    static GRBEnv env(true);
    env.set("LogToConsole", "0");
    env.set("LogFile", "gurobi.log");
    env.set("TimeLimit", "10");
    env.start();

    return env;
}
}  // namespace DSHunter
