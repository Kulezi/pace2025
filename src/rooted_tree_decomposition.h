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
        // parent_id equal to NONE means it's the root node.
        int parent_id;

        std::vector<int> bag;
        std::vector<int> children;

        DecompositionNode() : id(UNASSIGNED), parent_id(UNASSIGNED), bag(), children() {}
        DecompositionNode(int id, int parent_id, std::vector<int> bag, std::vector<int> children)
            : id(id), parent_id(parent_id), bag(bag), children(children) {}
    };

    int root;
    int width;
    std::vector<DecompositionNode> decomp;
    DecompositionNode &operator[](int v) { return decomp[v]; }
    const DecompositionNode &operator[](int v) const { return decomp[v]; }

    RootedTreeDecomposition(const TreeDecomposition &td)
        : root(0), width(td.width), decomp(td.size()) {
        if (td.size() > 0) {
            make_nodes(root, td, NONE);
            DS_ASSERT([&]() {
                // Check if depth first search filled all nodes.
                for (auto &node : decomp) {
                    if (node.id == UNASSIGNED) return false;
                    if (node.parent_id == UNASSIGNED) return false;
                }

                return true;
            }());
        } else {
            decomp = {DecompositionNode(0, NONE, {}, {})};
        }
    }

    RootedTreeDecomposition() = default;

    void SortBags() {
        for (auto &node : decomp) {
            sort(node.bag.begin(), node.bag.end());
        }
    }

    // Ensures bag content of JOIN children nodes equals the JOIN nodes' content,
    // By inserting an additional bag between it and its children.
    void EqualizeJoinChildren() { equalizeJoinChildren(root); }

    // Ensures each JOIN node has exactly two children.
    // Does so by inserting a new JOIN bag between a node with more than two children,
    // and any two children, reducing the degree by one each time until the tree is binary.
    void BinarizeJoins() { binarizeJoins(root); }

    // Inserts an empty bag above the root and below all the leaves.
    void ForceEmptyRootAndLeaves() {
        insertEmptyBagsUnderLeaves(root);

        int new_root = make_decomposition_node(NONE, {}, {root});
        decomp[root].parent_id = new_root;
        root = new_root;
    }

    int size() { return decomp.size(); }

    void print() {
        std::cerr << "decomposition of width " << width << " and size " << size() << std::endl;
        for (int i = 0; i < decomp.size(); i++) {
            std::cerr << i << ", bag = " << dbgv(decomp[i].bag)
                      << " children = " << dbgv(decomp[i].children)
                      << " parent = " << decomp[i].parent_id << std::endl;
        }

        std::cerr << std::endl;
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

    // Returns the index of the newly created node in decomp.
    int make_decomposition_node(int parent_id, std::vector<int> bag, std::vector<int> children) {
        int id = decomp.size();
        decomp.push_back(DecompositionNode{id, parent_id, bag, children});
        return id;
    }

    void equalizeJoinChildren(int node_id) {
        for (auto &child : decomp[node_id].children) {
            equalizeJoinChildren(child);

            // Insert a node with bag equal to to the join node between itself and the child.
            auto intermediate_node = make_decomposition_node(node_id, decomp[node_id].bag, {child});
            decomp[child].parent_id = intermediate_node;
            child = intermediate_node;
        }
    }

    void binarizeJoins(int node_id) {
        for (auto child : decomp[node_id].children) {
            binarizeJoins(child);
        }

        while (decomp[node_id].children.size() > 2) {
            auto l = decomp[node_id].children.back();
            decomp[node_id].children.pop_back();
            auto r = decomp[node_id].children.back();
            decomp[node_id].children.pop_back();

            auto intermediate_node = make_decomposition_node(node_id, decomp[node_id].bag, {l, r});
            decomp[l].parent_id = intermediate_node;
            decomp[r].parent_id = intermediate_node;
            decomp[node_id].children.push_back(intermediate_node);
        }
    }

    void insertEmptyBagsUnderLeaves(int node_id) {
        for (auto child : decomp[node_id].children) {
            insertEmptyBagsUnderLeaves(child);
        }

        if (decomp[node_id].children.empty()) {
            auto leaf = make_decomposition_node(node_id, {}, {});
            decomp[node_id].children.push_back(leaf);
        }
    }
};

#endif DS_ROOTED_TREE_DECOMPOSITION_H