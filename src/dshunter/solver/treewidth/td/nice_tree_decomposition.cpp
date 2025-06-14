#include "nice_tree_decomposition.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

#include "../../../utils.h"
namespace DSHunter {

NiceTreeDecomposition::NiceTreeDecomposition() = default;
NiceTreeDecomposition NiceTreeDecomposition::nicify(Instance g, TreeDecomposition td) {
    auto rooted_decomposition = RootedTreeDecomposition(td);
    rooted_decomposition.sortBags();
    rooted_decomposition.equalizeJoinChildren();
    rooted_decomposition.binarizeJoins();
    rooted_decomposition.forceEmptyRootAndLeaves();

    return NiceTreeDecomposition(g, rooted_decomposition);
}

NiceTreeDecomposition::NiceTreeDecomposition(Instance g,
                                             const RootedTreeDecomposition& rooted_decomposition)
    : g(g) {
    root = makeDecompositionNodeFromRootedDecomposition(rooted_decomposition,
                                                        rooted_decomposition.root)
               .first;
}

const NiceTreeDecomposition::Node& NiceTreeDecomposition::operator[](int v) const {
    return decomp[v];
}

int NiceTreeDecomposition::n_nodes() const { return decomp.size(); }

int NiceTreeDecomposition::width() const {
    int max_width = 0;
    for (auto& v : decomp)
        if (max_width < v.bag_size)
            max_width = v.bag_size;
    return max_width;
}

void NiceTreeDecomposition::print() {
    std::cerr << "n_nodes: " << n_nodes() << ", width: " << width() << "\n";
    printDecomp(root, 0);
}

int NiceTreeDecomposition::createNode(NodeType type, std::vector<int> bag, int v, int to, int lChild, int rChild) {
    int pos_v = -1;
    if (v != -1) {
        pos_v = std::ranges::lower_bound(bag, v) - bag.begin();
    }

    int pos_to = -1;
    if (v != -1) {
        pos_to = std::ranges::lower_bound(bag, to) - bag.begin();
    }

    decomp.push_back(Node{
        .id = static_cast<int>(decomp.size()),
        .type = type,
        .bag_size = static_cast<int>(bag.size()),
        .v = v,
        .to = to,
        .l_child = lChild,
        .r_child = rChild,
        .pos_v = pos_v,
        .pos_to = pos_to,
    });

    return decomp.back().id;
}

void NiceTreeDecomposition::printDecomp(int v, int level) {
    std::cerr << std::string(level, ' ');
    std::string vertex_label = "UNKNOWN";

    auto& node = decomp[v];
    switch (node.type) {
        case NodeType::Forget:
            vertex_label = "FORGET(" + std::to_string(node.v) + ")";
            break;
        case NodeType::IntroduceVertex:
            vertex_label = "INTRODUCE_VERTEX(" + std::to_string(node.v) + ")";
            break;
        case NodeType::IntroduceEdge:
            vertex_label =
                "INTRODUCE_EDGE(" + std::to_string(node.v) + ", " + std::to_string(node.to) + ")";
            break;
        case NodeType::Leaf:
            vertex_label = "LEAF";
            break;
        case NodeType::Join:
            vertex_label = "JOIN";
            break;
    }

    std::cerr << v << "." << vertex_label << " [";
    for (auto i : bag) std::cerr << " " << i;
    std::cerr << " ]\n";

    if (node.type != NodeType::Leaf) {
        if (node.type == NodeType::IntroduceVertex) {
            bag.erase(bag.begin() + node.pos_v);
        }
        if (node.type == NodeType::Forget) {
            bag.insert(bag.begin() + node.pos_v, node.v);
        }
        printDecomp(node.l_child, level + 1);
        if (node.type == NodeType::IntroduceVertex) {
            bag.insert(bag.begin() + node.pos_v, node.v);
        }
        if (node.type == NodeType::Forget) {
            bag.erase(bag.begin() + node.pos_v);
        }
    }
    if (node.type == NodeType::Join)
        printDecomp(node.r_child, level + 1);
}

std::pair<int, std::vector<int>> NiceTreeDecomposition::makeDecompositionNodeFromRootedDecomposition(
    const RootedTreeDecomposition& rtd, int rtd_node_id) {
    auto& rtd_node = rtd[rtd_node_id];
    int children_count = rtd_node.children.size();

    if (children_count == 0) {
        return { createNode(NodeType::Leaf), {} };
    }

    else if (children_count == 2) {
        DS_ASSERT(rtd_node.bag == rtd[rtd_node.children[0]].bag &&
                  rtd_node.bag == rtd[rtd_node.children[1]].bag);

        auto [l, l_bag] = makeDecompositionNodeFromRootedDecomposition(rtd, rtd[rtd_node_id].children[0]);
        auto [r, r_bag] = makeDecompositionNodeFromRootedDecomposition(rtd, rtd[rtd_node_id].children[1]);
        return { createNode(NodeType::Join, rtd_node.bag, NONE, NONE, l, r), l_bag };
    } else {
        DS_ASSERT(children_count == 1);

        auto [child, child_bag] = makeDecompositionNodeFromRootedDecomposition(rtd, rtd[rtd_node_id].children[0]);
        if (child_bag == rtd_node.bag)
            return { child, child_bag };

        return { makeIntroduceForgetSequenceFrom(rtd_node.bag, child_bag, child), rtd_node.bag };
    }
}

int NiceTreeDecomposition::makeIntroduceForgetSequenceFrom(const std::vector<int>& head_bag,
                                                           std::vector<int> tail_bag,
                                                           int tail_id) {
    auto intersection = intersect(head_bag, tail_bag);

    // Construct the sequence bottom-up.
    auto to_forget = remove(tail_bag, intersection);

    while (!to_forget.empty()) {
        int forgotten = to_forget.back();
        to_forget.pop_back();
        remove(tail_bag, forgotten);
        tail_id =
            createNode(NodeType::Forget, tail_bag, forgotten, NONE, tail_id);
    }

    DS_ASSERT(tail_bag == intersection);

    auto to_introduce = remove(head_bag, intersection);
    while (!to_introduce.empty()) {
        // First we introduce an isolated node.
        int introduced = to_introduce.back();
        to_introduce.pop_back();
        auto neighbours_in_bag = intersect(g[introduced].n_open, tail_bag);

        insert(tail_bag, introduced);
        tail_id = createNode(NodeType::IntroduceVertex, tail_bag, introduced, NONE, tail_id);

        // Then introduce each edge within the bag.
        for (auto to : neighbours_in_bag) {
            tail_id = createNode(NodeType::IntroduceEdge, tail_bag, introduced, to, tail_id);
        }
    }

    DS_ASSERT(tail_bag == head_bag);
    return tail_id;
}

}  // namespace DSHunter