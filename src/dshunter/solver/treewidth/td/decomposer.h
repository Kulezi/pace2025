#ifndef DS_DECOMPOSER_H
#define DS_DECOMPOSER_H
#include <optional>

#include "../../solver.h"
#include "tree_decomposition.h"

namespace DSHunter {
struct Decomposer {
    virtual ~Decomposer() = default;
    const SolverConfig* cfg;

    explicit Decomposer(const SolverConfig* cfg);

    // Finds a tree decomposition with approximately low treewidth.
    // Returns the first decomposition that will have treewidth under treewidth_threshold.
    // Note that time_limit only tells when to stop looking for new solutions, so it might
    // terminate a lot later.
    virtual std::optional<TreeDecomposition> decompose(const Instance& input_graph);
};

}  // namespace DSHunter

#endif  // DS_DECOMPOSER_H