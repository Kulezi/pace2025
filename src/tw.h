#ifndef _TW_H
#define _TW_H

#include <chrono>
#include <csignal>
#include <htd/main.hpp>
#include <htd_io/main.hpp>
#include <iostream>
#include <memory>

#include "ds.h"
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
struct TreeDecomposition {
    TreeDecomposition(Instance &g) {
        loadGraph(g);
        htdDecompose();
        // Rewrite the decomposition to fit our rules.
        rewriteDecomposition();
        printHtdDecomp(htdDecomposition->root(), 0);
        printDecomp(0, 0);
    };

    htd::IMutableGraph *graph;
    std::vector<int> reverse_mapping;
    htd::ITreeDecomposition *htdDecomposition;

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
        operation->setVertexSelectionStrategy(new htd::RandomVertexSelectionStrategy(10));
        operation->addManipulationOperation(new htd::NormalizationOperation(manager.get()));

        manager->orderingAlgorithmFactory().setConstructionTemplate(
            new htd::MinDegreeOrderingAlgorithm(manager.get()));

        htd::ITreeDecompositionAlgorithm *baseAlgorithm =
            manager->treeDecompositionAlgorithmFactory().createInstance();

        baseAlgorithm->addManipulationOperation(operation);
        htd::IterativeImprovementTreeDecompositionAlgorithm algorithm(manager.get(), baseAlgorithm,
                                                                      f.clone());

        algorithm.setIterationCount(10);
        algorithm.setNonImprovementLimit(3);
        htdDecomposition = algorithm.computeDecomposition(*graph);
    }

    enum NodeType { Introduce, Leaf, Forget, Join };
    struct DecompositionNode {
        int id;
        // Vertex number for accessing its decomposition bag.
        htd::vertex_t htd_id;
        NodeType type;

        // Either introduced or forgotten vertex.
        int v;
        int l_child;
        int r_child;
    };

    std::vector<DecompositionNode> decomp;
    const std::vector<htd::vertex_t> &bag(int v) {
        return htdDecomposition->bagContent(decomp[v].htd_id);
    }

    NodeType getNodeType(htd::vertex_t v) {
        if (htdDecomposition->isForgetNode(v)) return Forget;
        if (htdDecomposition->isLeaf(v)) return Leaf;
        if (htdDecomposition->isIntroduceNode(v)) return Introduce;
        if (htdDecomposition->isJoinNode(v)) return Join;
        throw std::logic_error("found decomposition node of unknown type");
    }

    // Returns the postorder of the decomp vector assigned to this decomposition node.
    void makeDecompositionNode(htd::vertex_t htd_id, int id) {
        decomp[id].id = id;
        decomp[id].htd_id = htd_id;

        decomp[id].type = getNodeType(htd_id);
        switch (decomp[id].type) {
            case Introduce:
                assert(htdDecomposition->introducedVertexCount(htd_id) == 1);
                decomp[id].v = htdDecomposition->introducedVertexAtPosition(htd_id, 0);
                decomp[id].l_child = id + 1;
                makeDecompositionNode(htdDecomposition->childAtPosition(htd_id, 0), id + 1);
                return;
            case Forget:
                assert(htdDecomposition->forgottenVertexCount(htd_id) == 1);
                decomp[id].v = htdDecomposition->forgottenVertexAtPosition(htd_id, 0);
                decomp[id].l_child = id + 1;
                makeDecompositionNode(htdDecomposition->childAtPosition(htd_id, 0), id + 1);
                return;
            case Leaf:
                assert(htdDecomposition->childCount(htd_id) == 0);
                return;
            case Join: {
                assert(htdDecomposition->childCount(htd_id) == 2);
                int htd_l_child = htdDecomposition->childAtPosition(htd_id, 0);
                int htd_r_child = htdDecomposition->childAtPosition(htd_id, 1);
                decomp[id].l_child = id + 1;
                decomp[id].r_child =
                    decomp[id].l_child + htdDecomposition->vertexCount(htd_l_child);
                makeDecompositionNode(htd_l_child, id + 1);
                makeDecompositionNode(htd_r_child, decomp[id].r_child);
                return;
            }
        }
    }

    void rewriteDecomposition() {
        int htd_root = htdDecomposition->root();
        decomp.resize(htdDecomposition->vertexCount(htd_root));
        makeDecompositionNode(htd_root, 0);
    }

    void printHtdDecomp(int v, int level) {
        std::cerr << std::string(level, ' ');
        std::string vertex_type = "UNKNOWN";

        if (htdDecomposition->isForgetNode(v)) {
            vertex_type = "FORGET";
            assert(htdDecomposition->bagSize(v) ==
                   htdDecomposition->bagSize(htdDecomposition->childAtPosition(v, 0)) - 1);
        } else if (htdDecomposition->isJoinNode(v)) {
            vertex_type = "JOIN";
            assert(htdDecomposition->childCount(v) == 2);
            assert(htdDecomposition->bagSize(v) ==
                   htdDecomposition->bagSize(htdDecomposition->childAtPosition(v, 0)));
            assert(htdDecomposition->bagSize(v) ==
                   htdDecomposition->bagSize(htdDecomposition->childAtPosition(v, 1)));
        } else if (htdDecomposition->isIntroduceNode(v)) {
            vertex_type = "INTRODUCE";
            if (htdDecomposition->children(v).empty())
                vertex_type = "LEAF";
            else {
                assert(htdDecomposition->bagSize(v) ==
                       htdDecomposition->bagSize(htdDecomposition->childAtPosition(v, 0)) + 1);
            }
        }
        std::cerr << "sz(" << htdDecomposition->vertexCount(v) << ") ";
        std::cerr << vertex_type << " " << v << ": [";
        for (auto i : htdDecomposition->bagContent(v)) std::cerr << " " << i;
        std::cerr << " ]\n";

        for (auto u : htdDecomposition->children(v)) {
            printHtdDecomp(u, level + 1);
        }
    };

    void printDecomp(int v, int level) {
        std::cerr << std::string(level, ' ');
        std::string vertex_type = "UNKNOWN";

        auto &node = decomp[v];
        switch (node.type) {
            case Forget:
                vertex_type = "FORGET";
                assert(bag(v).size() + 1 == bag(node.l_child).size());
                break;
            case Introduce:
                vertex_type = "INTRODUCE";
                assert(bag(v).size() == bag(node.l_child).size() + 1);
                break;
            case Leaf:
                vertex_type = "LEAF";
                break;
            case Join:
                vertex_type = "JOIN";
                assert(bag(v).size() == bag(node.l_child).size());
                assert(bag(v).size() == bag(node.r_child).size());
        }

        std::cerr << "sz(" << htdDecomposition->vertexCount(node.htd_id) << ") ";
        std::cerr << vertex_type << " " << v << ": [";
        for (auto i : bag(v)) std::cerr << " " << i;
        std::cerr << " ]\n";

        if (node.type != Leaf) printDecomp(node.l_child, level+1);
        if (node.type == Join) printDecomp(node.r_child, level+1);
    };

    ~TreeDecomposition() {
        delete htdDecomposition;
        delete graph;
    }
};
#endif  // _TW_H