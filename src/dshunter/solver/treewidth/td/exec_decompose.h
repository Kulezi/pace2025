#ifndef DS_EXEC_DECOMPOSE_H
#define DS_EXEC_DECOMPOSE_H
#include <chrono>
#include <optional>

#include "../../../instance.h"
#include "tree_decomposition.h"
namespace DSHunter {

// Finds a tree decomposition with approximately low treewidth.
// Returns the first decomposition that will have treewidth under treewidth_threshold.
// Note that time_limit only tells the FlowCutter to stop looking for new solutions, so it might
// terminate a lot later.
std::optional<TreeDecomposition> execDecompose(const std::string& path_to_decomposer,
                                               const Instance& input_graph,
                                               std::chrono::milliseconds time_limit);
// namespace DSHunter
}
#endif  // DS_EXEC_DECOMPOSE_H