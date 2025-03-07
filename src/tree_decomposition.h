#ifndef DS_TREE_DECOMPOSITION_H
#define DS_TREE_DECOMPOSITION_H

#include <chrono>
#include <csignal>
#include <iostream>
#include <memory>
#include <string>

#include "instance.h"
#include "rrules.h"

// Basic tree decomposition.
// Decomposition nodes are numbered from 0 to size() - 1.
// Node labels in the bag match the ones in Instance.
struct TreeDecomposition {
    int width;
    std::vector<std::vector<int>> bag;
    std::vector<std::vector<int>> adj;

    int size() const {
        return bag.size();
    }

    void print() const {
        std::cerr << "width: " << width << std::endl;
        std::cerr << "size: " << size() << std::endl;
        for (int i = 0; i < size(); i++) {
            std::cerr << i <<":" << dbgv(bag[i]) << dbgv(adj[i]) << std::endl;
        }
    }

    void addEdge(int a, int b) {
        DS_ASSERT(find(adj[a].begin(), adj[a].end(), b) == adj[a].end());
        DS_ASSERT(find(adj[b].begin(), adj[b].end(), a) == adj[b].end());
        adj[a].push_back(b);
        adj[b].push_back(a);
    }
};

#endif DS_TREE_DECOMPOSITION_H