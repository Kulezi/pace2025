#include <bits/stdc++.h>
#include "graph.h"
using namespace std;

int32_t main(int argc, char* argv[]) {
    assert(argc == 3);
    ifstream test(argv[1]);
    ifstream ds(argv[2]);

    Graph g(test);

    int n;
    ds >> n;


    vector<int> dominated(g.n_nodes + 1, 0);
    while (n--) {
        int v;
        ds >> v;

        dominated[v] = 1;
        for (auto u : g.adj[v]) dominated[u] = 1;
    }

    for (int i = 1; i <= g.n_nodes; i++) {
        assert(dominated[i]);
    }
}