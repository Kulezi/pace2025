#ifndef DS_GUROBI_SOLVER_H
#define DS_GUROBI_SOLVER_H
#include "../instance.h"

namespace DSHunter {

struct GurobiSolver {
    bool solve(Instance &g);
};
}  // namespace DSHunter
#endif