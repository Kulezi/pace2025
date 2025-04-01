#ifndef DS_TREE_DECOMPOSITION_H
#define DS_TREE_DECOMPOSITION_H

#include <iostream>

#include "instance.h"
#include "rrules.h"

namespace DSHunter {

// Basic tree decomposition.
// Decomposition nodes are numbered from 0 to size() - 1.
// Node labels in the bag match the ones in Instance.
struct TreeDecomposition {
    int width;
    std::vector<std::vector<int>> bag;
    std::vector<std::vector<int>> adj;

    int size() const;
    void print() const;

    void addEdge(int a, int b);
};
}  // namespace DSHunter
#endif  // DS_TREE_DECOMPOSITION_H