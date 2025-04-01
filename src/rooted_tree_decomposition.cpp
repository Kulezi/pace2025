#include "rooted_tree_decomposition.h"
#include "utils.h"
namespace DSHunter {

RootedTreeDecomposition::DecompositionNode &RootedTreeDecomposition::operator[](int v) {
    return decomp[v];
}

const RootedTreeDecomposition::DecompositionNode &RootedTreeDecomposition::operator[](int v) const {
    return decomp[v];
}

RootedTreeDecomposition::RootedTreeDecomposition(const DSHunter::TreeDecomposition &td)
    : root(0), width(td.width), decomp(td.size()) {
    if (td.size() > 0) {
        makeNodes(root, td, NONE);
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

void RootedTreeDecomposition::sortBags() {
    for (auto &node : decomp) {
        sort(node.bag.begin(), node.bag.end());
    }
}

// Ensures bag content of JOIN children nodes equals the JOIN nodes' content,
// By inserting an additional bag between it and its children.
void RootedTreeDecomposition::equalizeJoinChildren() { equalizeJoinChildren_(root); }

// Ensures each JOIN node has exactly two children.
// Does so by inserting a new JOIN bag between a node with more than two children,
// and any two children, reducing the degree by one each time until the tree is binary.
void RootedTreeDecomposition::binarizeJoins() { binarizeJoins_(root); }

// Inserts an empty bag above the root and below all the leaves.
void RootedTreeDecomposition::forceEmptyRootAndLeaves() {
    insertEmptyBagsUnderLeaves(root);

    int new_root = makeDecompositionNode(NONE, {}, {root});
    decomp[root].parent_id = new_root;
    root = new_root;
}

int RootedTreeDecomposition::size() { return decomp.size(); }

void RootedTreeDecomposition::print() {
    std::cerr << "decomposition of width " << width << " and size " << size() << std::endl;
    for (size_t i = 0; i < decomp.size(); i++) {
        std::cerr << i << ", bag = " << dbgv(decomp[i].bag)
                  << " children = " << dbgv(decomp[i].children)
                  << " parent = " << decomp[i].parent_id << std::endl;
    }

    std::cerr << std::endl;
}

void RootedTreeDecomposition::makeNodes(int u, const TreeDecomposition &td, int parent) {
    auto &node = decomp[u];
    node.id = u;
    node.parent_id = parent;
    node.bag = td.bag[u];
    for (auto v : td.adj[u]) {
        if (v != parent) {
            node.children.push_back(v);
            makeNodes(v, td, u);
        }
    }
}

// Returns the index of the newly created node in decomp.
int RootedTreeDecomposition::makeDecompositionNode(int parent_id, std::vector<int> bag,
                                                   std::vector<int> children) {
    int id = decomp.size();
    decomp.push_back(DecompositionNode{id, parent_id, bag, children});
    return id;
}

void RootedTreeDecomposition::equalizeJoinChildren_(int node_id) {
    for (auto &child : decomp[node_id].children) {
        equalizeJoinChildren_(child);

        // Insert a node with bag equal to to the join node between itself and the child.
        auto intermediate_node = makeDecompositionNode(node_id, decomp[node_id].bag, {child});
        decomp[child].parent_id = intermediate_node;
        child = intermediate_node;
    }
}

void RootedTreeDecomposition::binarizeJoins_(int node_id) {
    for (auto child : decomp[node_id].children) {
        binarizeJoins_(child);
    }

    while (decomp[node_id].children.size() > 2) {
        auto l = decomp[node_id].children.back();
        decomp[node_id].children.pop_back();
        auto r = decomp[node_id].children.back();
        decomp[node_id].children.pop_back();

        auto intermediate_node = makeDecompositionNode(node_id, decomp[node_id].bag, {l, r});
        decomp[l].parent_id = intermediate_node;
        decomp[r].parent_id = intermediate_node;
        decomp[node_id].children.push_back(intermediate_node);
    }
}

void RootedTreeDecomposition::insertEmptyBagsUnderLeaves(int node_id) {
    for (auto child : decomp[node_id].children) {
        insertEmptyBagsUnderLeaves(child);
    }

    if (decomp[node_id].children.empty()) {
        auto leaf = makeDecompositionNode(node_id, {}, {});
        decomp[node_id].children.push_back(leaf);
    }
}
}  // namespace DSHunter
