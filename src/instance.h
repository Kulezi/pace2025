#ifndef _INSTANCE_H
#define _INSTANCE_H
#include <algorithm>
#include <cassert>
#include <htd/main.hpp>
#include <iostream>
#include <list>
#include <queue>
#include <sstream>
#include <string>
#include <vector>
#include "setops.h"
#include <unordered_map>
// #include <htd/Graph.hpp>
// #include <htd/Helpers.hpp>
// #include <htd/GraphFactory.hpp>
using std::vector;

enum Status { UNDOMINATED, DOMINATED, TAKEN };

// Create a management instance of the 'htd' library in order to allow centralized configuration.
std::unique_ptr<htd::LibraryInstance> manager(htd::createManagementInstance(htd::Id::FIRST));

// Undirected graph representing an instance of dominating set problem.
// Nodes are marked with a domination status.
// Node labels are assigned incrementally starting with 1.
struct Instance {
    htd::IMutableGraph *graph;
    std::string problem;

    // adj[v] = list of adjacent nodes sorted by increasing node id.
    // Order is maintained to make set union/intersection possible in O(|A| + |B|).
    std::vector<Status> status;

    // Extra vertices cannot be taken into the dominating set, we assume they mean if we take them
    // we should take all their neighbours instead.
    std::vector<bool> is_extra;

    // Nodes already removed from the graph considered as the dominating set candidates.
    std::vector<int> ds;

    // Constructs graph from input stream assuming DIMACS-like .gr format.
    Instance(std::istream &in) : ds{} {
        std::string line;
        size_t E = 0;
        while (std::getline(in, line)) {
            std::stringstream tokens(line);
            std::string s;
            tokens >> s;
            if (s[0] == 'c') continue;
            if (s[0] == 'p') {
                int n_nodes;
                tokens >> problem >> n_nodes >> E;
                assert(problem == "ds");

                graph = manager->graphFactory().createInstance(n_nodes);
                status = vector(n_nodes + 1, UNDOMINATED);
                is_extra = vector(n_nodes + 1, false);
            } else {
                int a = stoi(s);
                int b;
                tokens >> b;

                add_edge(a, b);
            }
        }

        assert(E == graph->edgeCount());
    }

    size_t n_nodes() { return graph->vertexCount(); }


    Instance(Instance i, std::vector<htd::vertex_t> to_take) {
        *this = i;
        graph = graph->clone();

        auto vertices = graph->vertices();
        for (auto i : vertices) {
            if (find(to_take.begin(), to_take.end(), i) == to_take.end()) remove_node(i);
        }

        ds = {};
    }

    Instance clone() {
        Instance i = *this;
        i.graph = i.graph->clone();
        return i;
    }

    void set_status(int v, Status c) { status[v] = c; }

    Status get_status(int v) { return status[v]; }

    void take(int v) {
        assert(status[v] != TAKEN);
        std::cerr << "[";
        for (auto i : ds) std::cerr << i << " ";
        std::cerr << "]" << "+ " << v << std::endl;
        if (is_extra[v]) {
            for (auto u : neighbourhood_excluding(v)) {
                assert(get_status(u) != TAKEN);
                take(u);
            }

            return;
        }


        status[v] = TAKEN;

        ds.push_back(v);
        for (auto u : neighbourhood_excluding(v)) {
            assert(get_status(u) != TAKEN);
            set_status(u, DOMINATED);
        }

        remove_node(v);
    }

    htd::ConstCollection<htd::vertex_t> nodes() {
        return graph->vertices();
    }

    int deg(int v) { return graph->neighborCount(v); }


    // Adds an edge between nodes with id's u and v.
    // Complexity: O(deg(v)), due to maintaining adjacency list to be sorted.
    void add_edge(int u, int v) { graph->addEdge(u, v); }

    // Removes the node with given id.
    // Complexity: O(deg(v) + sum over deg(v) of neighbours)
    void remove_node(htd::vertex_t v) {
        if (!graph->isVertex(v)) return;
        graph->removeVertex(v);
    }

    void remove_nodes(std::vector<htd::vertex_t> &l) {
        for (auto &v : l) remove_node(v);
    }

    void remove_nodes(std::vector<htd::vertex_t> &&l) {
        for (auto &v : l) remove_node(v);
    }

    void remove_edge(int v, int w) { graph->removeEdge(v, w); }

    // Creates and returns the id of the created node.
    // Complexity: O(1)
    int add_node() {
        auto v = graph->addVertex();
        std::cerr << "[" << v << "]" << "\n";
        assert(v == status.size());
        status.push_back(UNDOMINATED);
        is_extra.push_back(true);
        return v;
    }

    std::vector<htd::vertex_t> neighbourhood_including(htd::vertex_t v) {
        auto n = graph->neighbors(v);
        std::vector res(n.begin(), n.end());
        insert(res, v);
        return res;
    }

    std::vector<htd::vertex_t> neighbourhood_excluding(htd::vertex_t v) {
        auto n = graph->neighbors(v);
        return std::vector(n.begin(), n.end());
    }

    bool has_edge(int u, int v) {
        return graph->isEdge(u, v);
    }

    void print() {
        std::cerr << "n = " << n_nodes() << ",\tm = " << graph->edgeCount() << "\n";
        for (auto i : graph->vertices()) {
            std::cerr << i << " color: " << get_status(i) << "\n";
        }
        for (auto i : graph->vertices()) {
            for (auto j : graph->neighbors(i)) {
                if (j > i) continue;
                std::cerr << i << " " << j << "\n";
            }
        }
    }

    int min_deg_node_of_status(Status s) {
        int best_v = -1;
        for (auto v : graph->vertices())
            if (get_status(v) == s && (best_v == -1 || deg(v) < deg(best_v))) best_v = v;

        return best_v;
    }

    // Splits the graph into connected components.
    vector<Instance> split() {
        vector<Instance> result;

        std::unordered_map<int, int> component;
        int components = 0;

        print();

        // Assign nodes to connected components using breadth-first search.
        for (auto v : graph->vertices()) {
            if (!component[v]) {
                std::vector<htd::vertex_t> to_take;
                component[v] = ++components;

                std::queue<int> q;
                q.push(v);
                while (!q.empty()) {
                    int w = q.front();
                    q.pop();

                    to_take.push_back(w);
                    for (auto u : graph->neighbors(w)) {
                        if (!component[u]) {
                            component[u] = component[w];
                            q.push(u);
                        }
                    }
                }


                std::cerr << "R"<< dbgv(to_take) << "\n";;
                result.emplace_back(*this, to_take);
            }
        }

        return result;
    }
};

#endif  // GRAPH_H