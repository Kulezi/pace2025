#ifndef TD_H
#define TD_H

#include <chrono>
#include <csignal>
#include <iostream>
#include <memory>
#include <string>

#include "instance.h"
#include "rrules.h"

enum class NodeType { IntroduceVertex, IntroduceEdge, Leaf, Forget, Join };
struct DecompositionNode {
    int id;
    NodeType type;
    std::vector<int> bag;

    // Either introduced or forgotten vertex, or -1 if it's not an Introduce node.
    int v;
    // Label of the other endpoint of an introduced edge or -1 if it's not an IntroduceEdge node.
    int to;
    int l_child;
    int r_child;
};

// Represents a tree decomposition rooted at node labeled 0.
struct TreeDecomposition {
   public:
    TreeDecomposition(Instance &g) : g(g) {

    };

    DecompositionNode &operator[](int v) { return decomp[v]; }
    int root;

   private:
    std::vector<int> reverse_mapping;

    Instance &g;
    std::vector<DecompositionNode> decomp;

    int createNode(NodeType type, std::vector<int> bag, int v = -1, int to = -1, int lChild = -1,
                   int rChild = -1) {
        decomp.push_back(DecompositionNode{
            .id = (int)decomp.size(),
            .type = type,
            .bag = bag,
            .v = v,
            .to = to,
            .l_child = lChild,
            .r_child = rChild,
        });

        return decomp.back().id;
    }



    void printDecomp(int v, int level) {
        std::cerr << std::string(level, ' ');
        std::string vertex_label = "UNKNOWN";

        auto &node = decomp[v];
        switch (node.type) {
            case NodeType::Forget:
                vertex_label = "FORGET(" + std::to_string(node.v) + ")";
                break;
            case NodeType::IntroduceVertex:
                vertex_label = "INTRODUCE_VERTEX(" + std::to_string(node.v) + ")";
                break;
            case NodeType::IntroduceEdge:
                vertex_label = "INTRODUCE_EDGE(" + std::to_string(node.v) + ", " +
                               std::to_string(node.to) + ")";
                break;
            case NodeType::Leaf:
                vertex_label = "LEAF";
                break;
            case NodeType::Join:
                vertex_label = "JOIN";
                break;
        }

        std::cerr << v << "." << vertex_label << " [";
        for (auto i : node.bag) std::cerr << " " << i;
        std::cerr << " ]\n";

        if (node.type != NodeType::Leaf) printDecomp(node.l_child, level + 1);
        if (node.type == NodeType::Join) printDecomp(node.r_child, level + 1);
    };

   public:
    int n_nodes() { return decomp.size(); }

    size_t width() {
        size_t max_width = 0;
        for (auto &v : decomp)
            if (max_width < v.bag.size()) max_width = v.bag.size();
        return max_width;
    }

    void print() {
        std::cerr << "n_nodes: " << n_nodes() << ", width: " << width() << "\n";
        printDecomp(root, 0);
    }
};
#endif  // TD_H