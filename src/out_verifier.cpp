#include <bits/stdc++.h>

#include "instance.h"
using namespace std;

int32_t main(int argc, char* argv[]) {
    assert(argc == 3);
    ifstream test(argv[1]);
    ifstream ds(argv[2]);

    Instance g(test);

    int n = 0;
    if (!(ds >> n)) {
        cerr << "DOMINATING SET NOT FOUND!" << endl;
        exit(1);
    }

    vector<int> dominated(g.nodeCount() + 1, 0);
    for (int i = 1; i <= n; i++) {
        int v = 0;
        ds >> v;

        dominated[v] = 1;
        for (auto u : g.adj[v]) dominated[u] = 1;
    }

    for (size_t i = 1; i <= g.nodeCount(); ++i) {
        if (!dominated[i]) {
            cerr << "node " << i << " undominated" << endl;
            exit(1);
        }
    }

    cout << "OK |D| = " << n << "\n";
}