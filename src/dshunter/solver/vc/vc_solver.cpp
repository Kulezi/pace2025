#include "vc_solver.h"

#include <stdio.h>
#include <string.h>

#include <iostream>
#include "../../utils.h"
#include "vc_lib.h"

namespace DSHunter {
std::vector<int> VCSolver::solve(Instance &g) {
    DS_ASSERT(g.edgeCount() == g.forcedEdgeCount());

    std::vector<int> rv(g.next_free_id);
    for (size_t i = 0; i < g.nodes.size(); i++) {
        int v = g.nodes[i];
        rv[v] = i;
    }
    
    std::vector<std::vector<int>> graph(g.nodeCount());
    for (size_t i = 0; i < g.nodes.size(); i++) {
        int v = g.nodes[i];
        for (auto [w, status] : g.adj[v]) {
            DS_ASSERT(status == FORCED);
            int j = rv[w];
            graph[i].push_back(j);
        }
    }

    std::vector<bool> MIS = getVC(graph);

    for (size_t i = 0; i < MIS.size(); i++) {
        if (!MIS[i]) {
            g.ds.push_back(g.nodes[i]);
        }
    }
    return g.ds;
}
}  // namespace DSHunter