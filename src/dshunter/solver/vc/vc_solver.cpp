#include "vc_solver.h"

#include <stdio.h>
#include <string.h>

#include <iostream>

#include "../../utils.h"
#include "solve_mwc.h"

namespace DSHunter {
std::vector<int> VCSolver::solve(Instance &g) {
    DS_ASSERT(g.edgeCount() == g.forcedEdgeCount());

    std::vector<int> rv(g.all_nodes.size());
    for (size_t i = 0; i < g.nodes.size(); i++) {
        int v = g.nodes[i];
        rv[v] = i;
    }

    std::vector<std::vector<int>> graph(g.nodeCount());
    for (size_t i = 0; i < g.nodes.size(); i++) {
        int v = g.nodes[i];
        for (auto [w, status] : g[v].adj) {
            DS_ASSERT(status == EdgeStatus::FORCED);
            int j = rv[w];
            graph[i].push_back(j);
        }
    }

    std::vector<int> VC = PACE2019::VC(graph);

    for (auto i : VC) {
            g.ds.push_back(g.nodes[i]);
    }

    return g.ds;
}
}  // namespace DSHunter