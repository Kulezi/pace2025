#ifndef DS_EXEC_DECOMPOSER_H
#define DS_EXEC_DECOMPOSER_H
#include "decomposer.h"

namespace DSHunter {
struct ExecDecomposer : Decomposer {
    using Decomposer::Decomposer;
    std::optional<TreeDecomposition> decompose(const Instance& input_graph) override;
};
}  // namespace DSHunter

#endif  // DS_EXEC_DECOMPOSER_H