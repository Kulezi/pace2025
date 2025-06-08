#ifndef DS_VC_SOLVER_H
#define DS_VC_SOLVER_H
#include "../../instance.h"

namespace DSHunter {

struct VCSolver {
    static std::vector<int> solve(Instance &g);
};
}  // namespace DSHunter
#endif