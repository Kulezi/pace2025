#ifndef _GRAPH_H
#define _GRAPH_H
#include <algorithm>
#include <cassert>
#include <list>
#include <sstream>
#include <string>
#include <vector>

enum Color { UNDOMINATED, DOMINATED, TAKEN };
using std::vector;
// Undirected graph representing an instance of dominating set problem, with colors regarding domination status assigned to nodes.
// Assumes that vertex numbers are assigned incrementally, without id reusage.
struct Graph {
    int n_nodes;
    int n_edges;
    int next_free_id;
    std::string problem;

    // List of nodes, sorted by increasing node id.
    std::list<int> nodes;

    // adj[v] = list of adjacent nodes sorted by increasing node id.
    // Order is maintained to make set union/intersection possible in O(|A| + |B|).
    vector<std::list<int>> adj;
    std::vector<Color> color;

    // Constructs graph from input stream assuming DIMACS-like .gr format.
    Graph(std::istream &in) : n_edges(0) {
        std::string line;
        int E = 0;
        while (std::getline(in, line)) {
            std::stringstream tokens(line);
            std::string s;
            tokens >> s;
            if (s[0] == 'c') continue;
            if (s[0] == 'p') {
                tokens >> problem >> n_nodes >> E;
                assert(problem == "ds");
                adj = vector(n_nodes + 1, std::list<int>());
                color = vector(n_nodes + 1, UNDOMINATED);
                for (int i = 1; i <= n_nodes; i++) {
                    nodes.push_back(i);
                }

                next_free_id = n_nodes + 1;
            } else {
                int a = stoi(s);
                int b;
                tokens >> b;

                add_edge(a, b);
            }
        }

        assert(E == n_edges);
    }

    void set_color(int v, Color c) { color[v] = c; }
    
    Color get_color(int v) { return color[v]; }

    int deg(int v) { return (int)adj[v].size(); }

    // Adds an edge between nodes with id's u and v.
    // Complexity: O(deg(v)), due to maintaining adjacency list to be sorted.
    void add_edge(int u, int v) {
        n_edges++;
        adj[u].merge({v});
        adj[v].merge({u});
    }

    // Removes the node with given id.
    // Complexity: O(deg(v) + sum over deg(v) of neighbours)
    void remove_node(int v) {
        assert(find(nodes.begin(), nodes.end(), v) != nodes.end());
        n_edges -= (int)adj[v].size();
        n_nodes--;
        for (auto u : adj[v]) {
            adj[u].remove(v);
        }
        adj[v].clear();

        nodes.remove(v);
    }

    void remove_nodes(std::list<int> &l) {
        for (auto &v : l) remove_node(v);
    }

    
    void remove_nodes(std::list<int> &&l) {
        for (auto &v : l) remove_node(v);
    }

    void remove_edge(int v, int w) {
        n_edges--;
        adj[v].remove(w);
        adj[w].remove(v);
    }

    // Creates and returns the id of the created node.
    // Complexity: O(1)
    int add_node() {
        adj.push_back({});
        nodes.push_back(next_free_id);
        color.push_back(UNDOMINATED);
        n_nodes++;
        return next_free_id++;
    }

    std::list<int> neighbourhood_including(int v) {
        auto res = adj[v];
        res.merge({v});
        return res;
    }


    std::list<int> neighbourhood_excluding(int v) { return adj[v]; }

    bool has_edge(int u, int v) {
        return std::find(adj[u].begin(), adj[u].end(), v) != adj[u].end();
    }

    void print() {
        std::cerr << "n = " << n_nodes << ",\tm = " << n_edges << "\n";
        for (int i = 1; i < next_free_id; i++) {
            std::cerr << i << " color: " << get_color(i) << "\n";
        }
        for (int i = 1; i < next_free_id; i++) {
            for (auto j : adj[i]) {
                if (j > i) continue;
                std::cerr << i << " " << j << "\n";
            }
        }
    }

    int min_deg_node_of_color(const int c) {
        int best_v = -1;
        for (auto v : nodes)
            if (get_color(v) == c && (best_v == -1 || deg(v) < deg(best_v)))
                best_v = v;

        return best_v;
    }
};

#endif  // GRAPH_H