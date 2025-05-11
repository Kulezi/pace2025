#ifndef DS_GUROBI_SOLVER_H
#define DS_GUROBI_SOLVER_H
#include <memory>
#include <optional>

#include "../../instance.h"
#include "gurobi_c++.h"

namespace DSHunter {

struct GurobiSolver {
    GurobiSolver(const Instance &g);

    std::optional<std::vector<int>> solve();
    int lowerBound();

private:
    static GRBEnv& getEnv();
    Instance g;
    std::vector<int> rv;
    GRBModel model;
    std::vector<GRBVar> is_selected;
};
}  // namespace DSHunter
#endif