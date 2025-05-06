#ifndef DS_HS_SOLVER_H
#define DS_HS_SOLVER_H
#include "../../instance.h"

namespace DSHunter {

struct HSSolver {
    std::vector<int> solve(Instance &g);
};
}  // namespace DSHunter
#endif