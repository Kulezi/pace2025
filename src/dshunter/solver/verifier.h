#ifndef DS_VERIFIER_H
#define DS_VERIFIER_H
#include "../instance.h"
namespace DSHunter {

// Checks whether the given solution is a valid dominating set of the given instance.
// Throws std::logic_error if it isn't.
void verify_solution(const Instance &g, const std::vector<int> &solution);
}  // namespace DSHunter
#endif  // DS_VERIFIER_H