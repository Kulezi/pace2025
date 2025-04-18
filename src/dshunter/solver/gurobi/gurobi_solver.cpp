#include "gurobi_solver.h"

#include "../../utils.h"
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

        std::vector<int> rv(g.all_nodes.size());
        for (size_t i = 0; i < g.nodes.size(); i++) {
            int v = g.nodes[i];
            rv[v] = i;
        }

        std::vector<GRBVar> is_selected(g.nodes.size());
        for (size_t i = 0; i < g.nodes.size(); ++i) {
            is_selected[i] =
                m.addVar(0.0, 1.0, 0.0, GRB_BINARY, "is_selected_" + std::to_string(g.nodes[i]));
        }

        for (int v : g.nodes) {
            GRBLinExpr node_constraint = 0;
            for (auto [u, status] : g[v].adj) {
                node_constraint += is_selected[rv[u]];

                if (status == EdgeStatus::FORCED) {
                    GRBLinExpr edge_constraint = 0;
                    edge_constraint += is_selected[rv[u]];
                    edge_constraint += is_selected[rv[v]];
                    m.addConstr(edge_constraint >= 1);
                }
            }
            node_constraint += is_selected[rv[v]];
            m.addConstr(node_constraint >= 1);
        }

        GRBLinExpr obj = 0;
        for (int v : g.nodes) {
            obj += is_selected[rv[v]];
        }

        m.setObjective(obj, GRB_MINIMIZE);

        m.optimize();

        if (m.get(GRB_IntAttr_Status) != GRB_OPTIMAL)
            return false;
        for (size_t i = 0; i < g.nodes.size(); ++i) {
            if (is_selected[i].get(GRB_DoubleAttr_X) > 0)
                g.ds.push_back(g.nodes[i]);
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
