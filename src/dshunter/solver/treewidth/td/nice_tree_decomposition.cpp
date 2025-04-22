#include "nice_tree_decomposition.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

#include "../../../utils.h"
#include "exec_decompose.h"
#include "flow_cutter_wrapper.h"
namespace DSHunter {

std::optional<NiceTreeDecomposition> NiceTreeDecomposition::decompose(
    const Instance& g, std::chrono::seconds decomposition_time_budget, int tw_good_enough, int tw_max, std::string decomposer_path) {
    TreeDecomposition initial_decomposition;
    if (decomposer_path.empty()) {
        initial_decomposition = FlowCutter::decompose(g, 0, decomposition_time_budget, tw_good_enough);
    } else {
        auto t = execDecompose("/home/dvdpawcio/repos/magisterka/PACE2017-TrackA/tw-heuristic", g, decomposition_time_budget);
        if (!t.has_value()) return std::nullopt;
        initial_decomposition = t.value();
    }

    if (initial_decomposition.width > tw_max)
        return std::nullopt;

    auto rooted_decomposition = RootedTreeDecomposition(initial_decomposition);
    rooted_decomposition.sortBags();
    rooted_decomposition.equalizeJoinChildren();
    rooted_decomposition.binarizeJoins();
    rooted_decomposition.forceEmptyRootAndLeaves();

    return NiceTreeDecomposition(g, rooted_decomposition);
}

NiceTreeDecomposition::NiceTreeDecomposition(Instance g,
                                             const RootedTreeDecomposition& rooted_decomposition)
    : g(g), decomp() {
    root = makeDecompositionNodeFromRootedDecomposition(rooted_decomposition,
                                                        rooted_decomposition.root);
}

const NiceTreeDecomposition::Node& NiceTreeDecomposition::operator[](int v) const {
    return decomp[v];
}

int NiceTreeDecomposition::n_nodes() const { return decomp.size(); }

size_t NiceTreeDecomposition::width() const {
    size_t max_width = 0;
    for (auto& v : decomp)
        if (max_width < v.bag.size())
            max_width = v.bag.size();
    return max_width;
}

void NiceTreeDecomposition::print() const {
    std::cerr << "n_nodes: " << n_nodes() << ", width: " << width() << "\n";
    printDecomp(root, 0);
}

int NiceTreeDecomposition::createNode(NodeType type, std::vector<int> bag, int v, int to, int lChild, int rChild) {
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

void NiceTreeDecomposition::printDecomp(int v, int level) const {
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
    for (auto i : node.bag) std::cerr << " " << i;
    std::cerr << " ]\n";

    if (node.type != NodeType::Leaf)
        printDecomp(node.l_child, level + 1);
    if (node.type == NodeType::Join)
        printDecomp(node.r_child, level + 1);
}

int NiceTreeDecomposition::makeDecompositionNodeFromRootedDecomposition(
    const RootedTreeDecomposition& rtd, int rtd_node_id) {
    auto& rtd_node = rtd[rtd_node_id];
    int children_count = rtd_node.children.size();

    if (children_count == 0) {
        return createNode(NiceTreeDecomposition::NodeType::Leaf);
    }

    else if (children_count == 2) {
        DS_ASSERT(rtd_node.bag == rtd[rtd_node.children[0]].bag &&
                  rtd_node.bag == rtd[rtd_node.children[1]].bag);

        int l = makeDecompositionNodeFromRootedDecomposition(rtd, rtd[rtd_node_id].children[0]);
        int r = makeDecompositionNodeFromRootedDecomposition(rtd, rtd[rtd_node_id].children[1]);
        return createNode(NiceTreeDecomposition::NodeType::Join, rtd_node.bag, NONE, NONE, l, r);
    } else {
        DS_ASSERT(children_count == 1);

        int child = makeDecompositionNodeFromRootedDecomposition(rtd, rtd[rtd_node_id].children[0]);
        if (decomp[child].bag == rtd_node.bag)
            return child;

        return makeIntroduceForgetSequenceFrom(rtd_node.bag, decomp[child].bag, child);
    }
}

int NiceTreeDecomposition::makeIntroduceForgetSequenceFrom(std::vector<int> head_bag,
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
            createNode(NiceTreeDecomposition::NodeType::Forget, tail_bag, forgotten, NONE, tail_id);
    }

    DS_ASSERT(tail_bag == intersection);

    auto to_introduce = remove(head_bag, intersection);
    while (!to_introduce.empty()) {
        // First we introduce an isolated node.
        int introduced = to_introduce.back();
        to_introduce.pop_back();
        auto neighbours_in_bag = intersect(g.neighbourhoodExcluding(introduced), tail_bag);

        insert(tail_bag, introduced);
        tail_id = createNode(NiceTreeDecomposition::NodeType::IntroduceVertex, tail_bag, introduced, NONE, tail_id);

        // Then introduce each edge within the bag.
        for (auto to : neighbours_in_bag) {
            tail_id = createNode(NiceTreeDecomposition::NodeType::IntroduceEdge, tail_bag, introduced, to, tail_id);
        }
    }

    DS_ASSERT(tail_bag == head_bag);
    return tail_id;
}

}  // namespace DSHunter