#include <chrono>
#include <iostream>

#include "ds.h"
#include "tree_decomposition.h"
#include "rooted_tree_decomposition.h"
#include "flow_cutter_wrapper.h"
#include "instance.h"

#include "rrules.h"
int main() {
    Instance g(std::cin);
    TreeDecomposition td = FlowCutter::decompose(g, 0, chrono::milliseconds(1s));
    cout << td.width << endl;
    for (int i = 0; i < td.bag.size(); i++) {
        cerr << "bag(" << i << ")" << " = " << dbgv(td.bag[i]) << endl;
    }

    for (int i = 0; i < td.adj.size(); i++) {
        for (auto j : td.adj[i]) {
            cerr << i << " " << j << endl;
        }
    }

    RootedTreeDecomposition rtd(td);
    
}
