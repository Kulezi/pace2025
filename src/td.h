#ifndef TD_H
#define TD_H

#include <chrono>
#include <csignal>
#include <htd/main.hpp>
#include <htd_io/main.hpp>
#include <iostream>
#include <memory>
#include <string>

#include "instance.h"
#include "rrules.h"

class FitnessFunction : public htd::ITreeDecompositionFitnessFunction {
   public:
    FitnessFunction(void) {}
    ~FitnessFunction() {}
    htd::FitnessEvaluation *fitness(const htd::IMultiHypergraph &graph,
                                    const htd::ITreeDecomposition &decomposition) const {
        HTD_UNUSED(graph)
        /**
         * Here we specify the fitness evaluation for a given decomposition.
         * In this case, we select the maximum bag size and the height.
         */
        return new htd::FitnessEvaluation(2, -(double)(decomposition.maximumBagSize()),
                                          -(double)(decomposition.height()));
    }

    FitnessFunction *clone(void) const { return new FitnessFunction(); }
};

std::unique_ptr<htd::LibraryInstance> manager(htd::createManagementInstance(htd::Id::FIRST));
enum class HtdNodeType { Introduce, Leaf, Forget, Join };
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
        loadGraph(g);
        htdDecompose();
        // Rewrite the decomposition so it fits the definition from Platypus Book.
        makeDecomposition();
        delete htd_decomposition;
        delete graph;
    };

    DecompositionNode &operator[](int v) { return decomp[v]; }
    int root;

   private:
    htd::IMutableGraph *graph;
    std::vector<int> reverse_mapping;
    htd::ITreeDecomposition *htd_decomposition;
    Instance &g;
    std::vector<DecompositionNode> decomp;

    void loadGraph(Instance &g) {
        // Dummy value for 1-indexing.
        reverse_mapping = {0};

        int N = 0;
        std::map<int, htd::vertex_t> mapping;
        for (auto v : g.nodes) {
            mapping[v] = ++N;
            reverse_mapping.push_back(v);
        }

        graph = manager->graphFactory().createInstance(N);
        for (auto u : g.nodes)
            for (auto v : g.adj[u])
                if (u < v) graph->addEdge(mapping[u], mapping[v]);
    }

    void htdDecompose() {
        FitnessFunction f;
        htd::TreeDecompositionOptimizationOperation *operation =
            new htd::TreeDecompositionOptimizationOperation(manager.get(), f.clone());

        operation->setManagementInstance(manager.get());
        operation->setVertexSelectionStrategy(new htd::RandomVertexSelectionStrategy(100));
        operation->addManipulationOperation(new htd::NormalizationOperation(manager.get()));

        manager->orderingAlgorithmFactory().setConstructionTemplate(
            new htd::MinDegreeOrderingAlgorithm(manager.get()));

        htd::ITreeDecompositionAlgorithm *baseAlgorithm =
            manager->treeDecompositionAlgorithmFactory().createInstance();

        baseAlgorithm->addManipulationOperation(operation);
        htd::IterativeImprovementTreeDecompositionAlgorithm algorithm(manager.get(), baseAlgorithm,
                                                                      f.clone());

        algorithm.setIterationCount(0);
        algorithm.setNonImprovementLimit(3);
        htd_decomposition = algorithm.computeDecomposition(*graph);
    }

    HtdNodeType getNodeType(htd::vertex_t v) {
        if (htd_decomposition->isForgetNode(v)) return HtdNodeType::Forget;
        if (htd_decomposition->isLeaf(v)) return HtdNodeType::Leaf;
        if (htd_decomposition->isIntroduceNode(v)) return HtdNodeType::Introduce;
        if (htd_decomposition->isJoinNode(v)) return HtdNodeType::Join;
        throw std::logic_error("found decomposition node of unknown type");
    }

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

    int makeDecompositionNode(htd::vertex_t htd_id) {
        HtdNodeType type = getNodeType(htd_id);
        std::vector<int> htd_bag = std::vector<int>(htd_decomposition->bagContent(htd_id).begin(),
                                                    htd_decomposition->bagContent(htd_id).end());
        for (auto &v : htd_bag) v = reverse_mapping[v];

        switch (type) {
            case HtdNodeType::Introduce: {
                assert(htd_decomposition->introducedVertexCount(htd_id) == 1);

                int lChild = makeDecompositionNode(htd_decomposition->childAtPosition(htd_id, 0));
                int v = reverse_mapping[htd_decomposition->introducedVertexAtPosition(htd_id, 0)];

                return createNode(NodeType::IntroduceVertex, htd_bag, v, -1, lChild);
            }
            case HtdNodeType::Forget: {
                assert(htd_decomposition->forgottenVertexCount(htd_id) == 1);

                int lChild = makeDecompositionNode(htd_decomposition->childAtPosition(htd_id, 0));
                int v = reverse_mapping[htd_decomposition->forgottenVertexAtPosition(htd_id, 0)];

                // For each disappearing edge insert an IntroduceEdge node between Forget and its
                // direct child.
                for (auto to : htd_bag) {
                    if (g.hasEdge(v, to)) {
                        lChild =
                            createNode(NodeType::IntroduceEdge, decomp[lChild].bag, v, to, lChild);
                    }
                }

                return createNode(NodeType::Forget, htd_bag, v, -1, lChild);
            }

            case HtdNodeType::Leaf: {
                assert(htd_decomposition->childCount(htd_id) == 0);

                // htd provides a decomposition where leaves can have nonempty bags,
                // so replace a bag of size n with n introduces ending with a leaf.
                std::vector<int> bag;
                int last = createNode(NodeType::Leaf, {});

                for (auto i : htd_bag) {
                    bag.push_back(i);
                    last = createNode(NodeType::IntroduceVertex, bag, i, -1, last);
                }

                return last;
            }
            case HtdNodeType::Join: {
                assert(htd_decomposition->childCount(htd_id) == 2);

                int htd_l_child = htd_decomposition->childAtPosition(htd_id, 0);
                int htd_r_child = htd_decomposition->childAtPosition(htd_id, 1);

                int l_child = makeDecompositionNode(htd_l_child);
                int r_child = makeDecompositionNode(htd_r_child);

                return createNode(NodeType::Join, htd_bag, -1, -1, l_child, r_child);
            }

            default:
                throw std::logic_error("unknown HtdNodeType encountered");
        }
    }

    // Returns the id of the root of decomposition.
    void makeDecomposition() {
        int htd_root = htd_decomposition->root();
        root = makeDecompositionNode(htd_root);

        // The root bag might not be empty yet, so we forget vertices one by one, possibly
        // introducing new edges.
        while (!decomp[root].bag.empty()) {
            auto new_bag = decomp[root].bag;
            int forgotten = new_bag.back();
            new_bag.pop_back();

            // For each disappearing edge insert an IntroduceEdge node between Forget and its
            // direct child.
            for (auto to : new_bag) {
                if (g.hasEdge(forgotten, to)) {
                    root =
                        createNode(NodeType::IntroduceEdge, decomp[root].bag, forgotten, to, root);
                }
            }

            root = createNode(NodeType::Forget, new_bag, forgotten, -1, root);
        }
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