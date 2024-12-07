#include <bits/stdc++.h>

#include "instance.h"
using namespace std;

int32_t main(int argc, char* argv[]) {
    assert(argc == 3);
    ifstream test(argv[1]);
    ifstream ds(argv[2]);

    Instance g(test);

    int n;
    if (!(ds >> n)) {
        cerr << "DOMINATING SET NOT FOUND!" << endl;
        exit(1);
    }

    vector<int> dominated(g.n_nodes + 1, 0);
    for (int i = 1; i <= n; i++) {
        int v;
        ds >> v;

        dominated[v] = 1;
        for (auto u : g.adj[v]) dominated[u] = 1;
    }

    for (int i = 1; i <= g.n_nodes; i++) {
        if (!dominated[i]) {
            cerr << "node " << i << " undominated" << endl;
            exit(1);
        }
    }

    cout << "OK |D| = " << n << "\n";
}