// Original source:
// https://github.com/kit-algo/flow-cutter-pace17/blob/7f94541b0119284ea9322d528cef420e041539b6/src/pace.cpp
// Adapted for the purposes of PACE2025 Dominating Set Exact Solver.

#ifndef DS_FLOW_CUTTER_WRAPPER_H
#define DS_FLOW_CUTTER_WRAPPER_H
#include <chrono>

#include "../../../instance.h"
#include "tree_decomposition.h"
namespace DSHunter::FlowCutter {

// Finds a tree decomposition with approximately low treewidth.
// Returns the first decomposition that will have treewidth under treewidth_threshold.
// Note that time_limit only tells the FlowCutter to stop looking for new solutions, so it might
// terminate a lot later.
DSHunter::TreeDecomposition decompose(DSHunter::Instance input_graph, int random_seed,
                                      std::chrono::milliseconds time_limit,
                                      int treewidth_threshold);
}  // namespace DSHunter::FlowCutter

#endif  // DS_FLOW_CUTTER_WRAPPER_H