#ifndef _INSTANCE_H
#define _INSTANCE_H
#include <algorithm>
#include <cassert>
#include <iostream>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

#include "setops.h"

enum Status { UNDOMINATED, DOMINATED, TAKEN };

// Undirected graph representing an instance of dominating set problem.
// Nodes are marked with a domination status.
// Node labels are assigned incrementally starting with 1.
struct Instance {
    int n_edges;
    int next_free_id;

    // List of nodes, sorted by increasing node id.
    std::vector<int> nodes;

    // adj[v] = list of adjacent nodes sorted by increasing node id.
    // Order is maintained to make set union/intersection possible in O(|A| + |B|).
    std::vector<std::vector<int>> adj;
    std::vector<Status> status;

    // Extra vertices cannot be taken into the dominating set, we assume they mean if we take them
    // we should take all their neighbours instead.
    std::vector<bool> is_extra;

    // Nodes already removed from the graph considered as the dominating set candidates.
    std::vector<int> ds;

    // Constructs graph from input stream assuming DIMACS-like .gr format.
    Instance(std::istream &in) : n_edges(0), ds{} {
        std::string line;
        int E = 0;
        while (std::getline(in, line)) {
            std::stringstream tokens(line);
            std::string s;
            tokens >> s;
            if (s[0] == 'c') continue;
            if (s[0] == 'p') {
                parse_header(tokens, E);
            } else {
                int a = stoi(s);
                int b;
                tokens >> b;

                add_edge(a, b);
            }
        }

        assert(E == n_edges);
    }

    void parse_header(std::stringstream &tokens, int &E) {
        std::string problem;
        int n_nodes;
        tokens >> problem >> n_nodes >> E;
        assert(problem == "ds");
        adj.resize(n_nodes + 1);
        status.resize(n_nodes + 1, UNDOMINATED);
        is_extra.resize(n_nodes + 1, false);
        for (int i = 1; i <= n_nodes; ++i) {
            nodes.push_back(i);
        }
        next_free_id = n_nodes + 1;
    }

    Instance(Instance i, std::vector<int> to_take) {
        *this = i;
        sort(to_take.begin(), to_take.end());
        nodes = to_take;
        n_edges = 0;
        for (auto v : nodes) n_edges += adj[v].size();
        n_edges /= 2;
        ds = {};
    }

    int n_nodes() { return nodes.size(); }

    void set_status(int v, Status c) { status[v] = c; }

    Status get_status(int v) { return status[v]; }

    int deg(int v) { return (int)adj[v].size(); }

    // Creates and returns the id of the created node.
    // Complexity: O(1)
    int add_node() {
        adj.push_back({});
        nodes.push_back(next_free_id);
        status.push_back(UNDOMINATED);
        is_extra.push_back(true);
        return next_free_id++;
    }

    // Removes the node with given id.
    // Complexity: O(deg(v) + sum over deg(v) of neighbours)
    void remove_node(int v) {
        if (find(nodes.begin(), nodes.end(), v) == nodes.end()) return;
        n_edges -= (int)adj[v].size();
        for (auto u : adj[v]) {
            remove(adj[u], v);
            assert(is_sorted(adj[u].begin(), adj[u].end()));
        }
        adj[v].clear();

        remove(nodes, v);
        assert(is_sorted(nodes.begin(), nodes.end()));
    }

    void remove_nodes(const std::vector<int> &l) {
        for (auto &v : l) remove_node(v);
    }

    // Adds an edge between nodes with id's u and v.
    // Complexity: O(deg(v)), due to maintaining adjacency list to be sorted.
    void add_edge(int u, int v) {
        n_edges++;
        insert(adj[u], v);
        insert(adj[v], u);
    }

    void remove_edge(int v, int w) {
        n_edges--;
        remove(adj[v], w);
        remove(adj[w], v);
    }

    std::vector<int> neighbourhood_including(int v) {
        auto res = adj[v];
        insert(res, v);
        return res;
    }

    std::vector<int> neighbourhood_excluding(int v) { return adj[v]; }

    bool has_edge(int u, int v) {
        return std::find(adj[u].begin(), adj[u].end(), v) != adj[u].end();
    }

    int min_deg_node_of_status(Status s) {
        int best_v = -1;
        for (auto v : nodes)
            if (get_status(v) == s && (best_v == -1 || deg(v) < deg(best_v))) best_v = v;

        return best_v;
    }

    void take(int v) {
        assert(status[v] != TAKEN);
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

    // Splits the graph into connected components.
    std::vector<Instance> split() {
        std::vector<Instance> result;

        std::vector<int> component(next_free_id + 1, 0);
        int components = 0;

        // Assign nodes to connected components using breadth-first search.
        for (auto v : nodes) {
            if (!component[v]) {
                std::vector<int> to_take;
                component[v] = ++components;

                std::queue<int> q;
                q.push(v);
                while (!q.empty()) {
                    int w = q.front();
                    q.pop();

                    to_take.push_back(w);
                    for (auto u : adj[w]) {
                        if (!component[u]) {
                            component[u] = component[w];
                            q.push(u);
                        }
                    }
                }

                result.emplace_back(*this, to_take);
            }
        }

        return result;
    }

    void print() {
        std::cerr << "[n = " << n_nodes() << ",\tm = " << n_edges << "]\n";
        for (int i : nodes) {
            std::cerr << "color(" << i << ") = " << get_status(i) << "\n";
        }
        for (int i : nodes) {
            for (auto j : adj[i]) {
                if (j > i) continue;
                std::cerr << i << " " << j << "\n";
            }
        }
    }
};

#endif  // GRAPH_H