#ifndef DS_FLOW_CUTTER_DECOMPOSER_H
#define DS_FLOW_CUTTER_DECOMPOSER_H
#include "decomposer.h"

namespace DSHunter {
struct FlowCutterDecomposer : Decomposer {
    using Decomposer::Decomposer;
    std::optional<TreeDecomposition> decompose(const Instance& input_graph) override;
};
}  // namespace DSHunter

#endif  // DS_FLOW_CUTTER_DECOMPOSER_H