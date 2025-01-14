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
    htd::IMutableGraph *graph;
    std::vector<int> reverse_mapping;
    htd::ITreeDecomposition *decomposition;

    TreeDecomposition(Instance &g) {
        loadGraph(g);
        decompose();
        print_decomp(decomposition->root(), 0);
    };

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

    void decompose() {
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
        decomposition = algorithm.computeDecomposition(*graph);
    }

    void print_decomp(int v, int level) {
        std::cerr << std::string(level, ' ');
        std::string vertex_type = "UNKNOWN";

        if (decomposition->isForgetNode(v)) {
            vertex_type = "FORGET";
            assert(decomposition->bagSize(v) ==
                   decomposition->bagSize(decomposition->childAtPosition(v, 0)) - 1);
        } else if (decomposition->isJoinNode(v)) {
            vertex_type = "JOIN";
            assert(decomposition->childCount(v) == 2);
            assert(decomposition->bagSize(v) ==
                   decomposition->bagSize(decomposition->childAtPosition(v, 0)));
            assert(decomposition->bagSize(v) ==
                   decomposition->bagSize(decomposition->childAtPosition(v, 1)));
        } else if (decomposition->isIntroduceNode(v)) {
            vertex_type = "INTRODUCE";
            if (decomposition->children(v).empty())
                vertex_type = "LEAF";
            else {
                assert(decomposition->bagSize(v) ==
                       decomposition->bagSize(decomposition->childAtPosition(v, 0)) + 1);
            }
        }

        std::cerr << vertex_type << " " << v << ": [";
        for (auto i : decomposition->bagContent(v)) std::cerr << " " << i;
        std::cerr << " ]\n";

        for (auto u : decomposition->children(v)) {
            print_decomp(u, level + 1);
        }
    };

    ~TreeDecomposition() {
        delete decomposition;
        delete graph;
    }
};
#endif // _TW_H