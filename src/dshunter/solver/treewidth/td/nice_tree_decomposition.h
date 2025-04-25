#ifndef DS_NICE_TREE_DECOMPOSITION_H
#define DS_NICE_TREE_DECOMPOSITION_H

#include <chrono>
#include <optional>

#include "../../../instance.h"
#include "rooted_tree_decomposition.h"
namespace DSHunter {

enum class DecompositionError {
    NOT_ENOUGH_MEMORY,
    TOO_LARGE_TREEWIDTH
};

// Represents a tree decomposition rooted at node labeled 0.
struct NiceTreeDecomposition {
   public:
    enum class NodeType {
        IntroduceVertex,
        IntroduceEdge,
        Leaf,
        Forget,
        Join
    };
    
    struct Node {
        int id;
        NodeType type;
        std::vector<int> bag;

        int v;   // Introduced/forgotten vertex, or -1 if not applicable.
        int to;  // Other endpoint of an introduced edge, or -1 if not applicable.
        int l_child;
        int r_child;
    };

    static NiceTreeDecomposition nicify(
        const Instance& g, TreeDecomposition td);

    const Node& operator[](int v) const;
    int root;

    int n_nodes() const;
    int width() const;
    void print() const;

   private:
    const Instance g;
    std::vector<Node> decomp;
    static const int NONE = -1;

    // Assumes rooted_decomposition is already normalized!
    NiceTreeDecomposition(Instance g, const RootedTreeDecomposition& rooted_decomposition);
    int createNode(NodeType type, std::vector<int> bag = {}, int v = NONE, int to = NONE, int lChild = NONE, int rChild = NONE);

    void printDecomp(int v, int level) const;

    int makeDecompositionNodeFromRootedDecomposition(const RootedTreeDecomposition& rtd,
                                                     int rtd_node_id);

    int makeIntroduceForgetSequenceFrom(std::vector<int> head_bag, std::vector<int> tail_bag, int tail_id);
};

}  // namespace DSHunter

#endif  // DS_NICE_TREE_DECOMPOSITION_H