#ifndef DS_TREE_DECOMPOSITION_H
#define DS_TREE_DECOMPOSITION_H

#include <iostream>

#include "../../../instance.h"

namespace DSHunter {

// Basic tree decomposition.
// Decomposition nodes are numbered from 0 to size() - 1.
// Node labels in the bag match the ones in Instance.
struct TreeDecomposition {
    int width;
    std::vector<std::vector<int>> bag;
    std::vector<std::vector<int>> adj;

    [[nodiscard]] int size() const;
    void print() const;

    [[nodiscard]] int biggestBag() const;
    void removeNode(int v);

    void addEdge(int a, int b);
};
}  // namespace DSHunter
#endif  // DS_TREE_DECOMPOSITION_H