#ifndef DS_NICE_TREE_DECOMPOSITION_H
#define DS_NICE_TREE_DECOMPOSITION_H

#include <chrono>
#include <csignal>
#include <iostream>
#include <memory>
#include <string>

#include "flow_cutter_wrapper.h"
#include "instance.h"
#include "rooted_tree_decomposition.h"
#include "rrules.h"

// Represents a tree decomposition rooted at node labeled 0.
struct NiceTreeDecomposition {
   public:
    enum class NodeType { IntroduceVertex, IntroduceEdge, Leaf, Forget, Join };
    struct Node {
        int id;
        NodeType type;
        std::vector<int> bag;

        // Either introduced or forgotten vertex, or -1 if it's not an Introduce node.
        int v;
        // Label of the other endpoint of an introduced edge or -1 if it's not an IntroduceEdge
        // node.
        int to;
        int l_child;
        int r_child;
    };

    NiceTreeDecomposition(Instance &g, int treewidth_threshold) : g(g), decomp() {
        auto initial_decomposition = FlowCutter::decompose(g, 0, 1s, treewidth_threshold);

        auto rooted_decomposition = RootedTreeDecomposition(initial_decomposition);
        rooted_decomposition.SortBags();
        rooted_decomposition.EqualizeJoinChildren();
        rooted_decomposition.BinarizeJoins();
        rooted_decomposition.ForceEmptyRootAndLeaves();

        root = makeDecompositionNodeFromRootedDecomposition(rooted_decomposition,
                                                            rooted_decomposition.root);
    }

    Node &operator[](int v) { return decomp[v]; }
    int root;

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

   private:
    Instance &g;
    std::vector<Node> decomp;
    static const int NONE = -1;
    int createNode(NodeType type, std::vector<int> bag = {}, int v = NONE, int to = NONE,
                   int lChild = NONE, int rChild = NONE) {
        decomp.push_back(Node{
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
    }

    int makeDecompositionNodeFromRootedDecomposition(const RootedTreeDecomposition &rtd,
                                                     int rtd_node_id) {
        auto &rtd_node = rtd[rtd_node_id];
        int children_count = rtd_node.children.size();

        if (children_count == 0) {
            return createNode(NiceTreeDecomposition::NodeType::Leaf);
        }

        else if (children_count == 2) {
            DS_ASSERT(rtd_node.bag == rtd[rtd_node.children[0]].bag &&
                      rtd_node.bag == rtd[rtd_node.children[1]].bag);

            int l = makeDecompositionNodeFromRootedDecomposition(rtd, rtd[rtd_node_id].children[0]);
            int r = makeDecompositionNodeFromRootedDecomposition(rtd, rtd[rtd_node_id].children[1]);
            return createNode(NiceTreeDecomposition::NodeType::Join, rtd_node.bag, NONE, NONE, l,
                              r);
        } else {
            DS_ASSERT(children_count == 1);

            int child =
                makeDecompositionNodeFromRootedDecomposition(rtd, rtd[rtd_node_id].children[0]);
            if (decomp[child].bag == rtd_node.bag) return child;

            return makeIntroduceForgetSequenceFrom(rtd_node.bag, decomp[child].bag, child);
        }
    }

    // Returns the head of a sequence of Introduce/Forgets that transforms head_bag into tail_bag.
    // Graphically it is equal to:
    // (head_bag)
    // ... sequence of Introduce(Edge) ...
    // (head_bag ∪ tail_bag)
    // ... sequence of Forget ...
    // (tail_bag)
    int makeIntroduceForgetSequenceFrom(std::vector<int> head_bag, std::vector<int> tail_bag,
                                        int tail_id) {
        auto intersection = intersect(head_bag, tail_bag);

        // Construct the sequence bottom-up.
        auto to_forget = remove(tail_bag, intersection);

        while (!to_forget.empty()) {
            int forgotten = to_forget.back();
            to_forget.pop_back();
            remove(tail_bag, forgotten);
            tail_id = createNode(NiceTreeDecomposition::NodeType::Forget, tail_bag, forgotten, NONE,
                                 tail_id);
        }

        DS_ASSERT(tail_bag == intersection);

        auto to_introduce = remove(head_bag, intersection);
        while (!to_introduce.empty()) {
            // First we introduce an isolated node.
            int introduced = to_introduce.back();
            to_introduce.pop_back();
            auto neighbours_in_bag = intersect(g.adj[introduced], tail_bag);

            insert(tail_bag, introduced);
            tail_id = createNode(NiceTreeDecomposition::NodeType::IntroduceVertex, tail_bag,
                                 introduced, NONE, tail_id);

            // Then introduce each edge within the bag.
            for (auto to : neighbours_in_bag) {
                tail_id = createNode(NiceTreeDecomposition::NodeType::IntroduceEdge, tail_bag,
                                     introduced, to, tail_id);
            }
        }

        DS_ASSERT(tail_bag == head_bag);
        return tail_id;
    }
};
#endif  // NICE_TREE_DECOMPOSITION_H