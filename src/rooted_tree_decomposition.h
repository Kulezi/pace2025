#ifndef DS_ROOTED_TREE_DECOMPOSITION_H
#define DS_ROOTED_TREE_DECOMPOSITION_H

#include <chrono>
#include <csignal>
#include <iostream>
#include <memory>
#include <string>

#include "instance.h"
#include "rrules.h"
#include "tree_decomposition.h"
struct RootedTreeDecomposition {
    enum {
        UNASSIGNED = -2,
        NONE = -1,
    };

    struct DecompositionNode {
        int id;
        // parent_id equal to -1 means it's the root node.
        int parent_id;

        std::vector<int> bag;
        std::vector<int> children;

        DecompositionNode() : id(UNASSIGNED), parent_id(UNASSIGNED), bag(), children() {}
    };

    int root;
    int width;
    std::vector<DecompositionNode> decomp;

    RootedTreeDecomposition(const TreeDecomposition &td) : decomp(td.size()) {
        make_nodes(0, td, NONE);
        DS_ASSERT([&]() {
            // Check if depth first search filled all nodes.
            for (auto &node : decomp) {
                if (node.id == UNASSIGNED) return false;
                if (node.parent_id == UNASSIGNED) return false;
            }

            return true;
        }());
    }

   private:
    void make_nodes(int u, const TreeDecomposition &td, int parent) {
        auto &node = decomp[u];
        node.id = u;
        node.parent_id = parent;
        node.bag = td.bag[u];
        for (auto v : td.adj[u]) {
            if (v != parent) {
                node.children.push_back(v);
                make_nodes(v, td, u);
            }
        }
    }
};

#endif DS_ROOTED_TREE_DECOMPOSITION_H