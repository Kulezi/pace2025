#include "decomposer.h"
namespace DSHunter {
Decomposer::Decomposer(const SolverConfig* cfg) : cfg(cfg) {}

std::optional<TreeDecomposition> Decomposer::decompose(const Instance& input_graph) { return std::nullopt; }
}  // namespace DSHunter