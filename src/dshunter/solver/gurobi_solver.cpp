#include "gurobi_solver.h"

#include "gurobi_c++.h"
namespace DSHunter {
bool GurobiSolver::solve(Instance &g) {
    try {
        // Create an environment
        GRBEnv env = GRBEnv(true);
        env.set("LogToConsole", "0");
        env.set("LogFile", "gurobi.log");
        
        env.start();
        GRBModel m = GRBModel(env);
        m.set(GRB_DoubleParam_TimeLimit, 60.0);
        
        std::vector<GRBVar> is_selected(g.nodes.size());
        for (size_t i = 0; i < g.nodes.size(); ++i) {
            is_selected[i] =
                m.addVar(0.0, 1.0, 0.0, GRB_BINARY, "is_selected_" + std::to_string(g.nodes[i]));
        }

        for (int v : g.nodes) {
            GRBLinExpr node_constraint = 0;
            for (auto [u, status] : g.adj[v]) {
                node_constraint += is_selected[u - 1];

                if (status == FORCED) {
                    GRBLinExpr edge_constraint = 0;
                    edge_constraint += is_selected[u - 1];
                    edge_constraint += is_selected[v - 1];
                    m.addConstr(edge_constraint >= 1);
                }
            }
            node_constraint += is_selected[v - 1];
            m.addConstr(node_constraint >= 1);
        }

        GRBLinExpr obj = 0;
        for (int v : g.nodes) {
            obj += is_selected[v - 1];
        }

        m.setObjective(obj, GRB_MINIMIZE);

        m.optimize();

        for (size_t i = 0; i < g.nodes.size(); ++i) {
            if (is_selected[i].get(GRB_DoubleAttr_X) > 0) g.ds.push_back(g.nodes[i]);
        }

        return true;
    } catch (GRBException e) {
        std::cerr << "Error code = " << e.getErrorCode() << std::endl;
        std::cerr << e.getMessage() << std::endl;
    } catch (...) {
        std::cerr << "Exception during optimization" << std::endl;
    }

    return false;
}
}  // namespace DSHunter
