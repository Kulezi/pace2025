#ifndef DS_ROOTED_TREE_DECOMPOSITION_H
#define DS_ROOTED_TREE_DECOMPOSITION_H

#include "tree_decomposition.h"

#include <utility>
namespace DSHunter {

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

        DecompositionNode() : id(UNASSIGNED), parent_id(UNASSIGNED) {}
        DecompositionNode(int id, int parent_id, std::vector<int> bag, const std::vector<int> &children)
            : id(id), parent_id(parent_id), bag(std::move(bag)), children(children) {}
    };

    int root;
    int width;
    std::vector<DecompositionNode> decomp;
    DecompositionNode &operator[](int v);
    const DecompositionNode &operator[](int v) const;

    explicit RootedTreeDecomposition(const TreeDecomposition &td);

    RootedTreeDecomposition() = default;

    void sortBags();

    // Ensures bag content of JOIN children nodes equals the JOIN nodes' content,
    // By inserting an additional bag between it and its children.
    void equalizeJoinChildren();
    // Ensures each JOIN node has exactly two children.
    // Does so by inserting a new JOIN bag between a node with more than two children,
    // and any two children, reducing the degree by one each time until the tree is binary.
    void binarizeJoins();
    // Inserts an empty bag above the root and below all the leaves.
    void forceEmptyRootAndLeaves();

    int size() const;
    void print() const;

   private:
    void makeNodes(int u, const TreeDecomposition &td, int parent);
    int makeDecompositionNode(int parent_id, const std::vector<int>& bag, const std::vector<int>& children);
    void equalizeJoinChildren_(int node_id);
    void binarizeJoins_(int node_id);
    void insertEmptyBagsUnderLeaves(int node_id);
};
}  // namespace DSHunter
#endif  // DS_ROOTED_TREE_DECOMPOSITION_H