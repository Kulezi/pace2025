#include "vc_solver.h"

#include <stdio.h>
#include <string.h>

#include <iostream>
#include "../utils.h"

#include "graph_access.h"
#include "graph_io.h"
#include "timer.h"
#include "mis_permutation.h"
#include "exact_mis.h"
#include "ils.h"
#include "mis_config.h"
#include "mis_log.h"
namespace DSHunter {
std::vector<int> VCSolver::solve(Instance &g) {
    DS_ASSERT(g.edgeCount() == g.forcedEdgeCount());
    mis_log::instance()->restart_total_timer();

    MISConfig mis_config;

    // Parse the command line parameters;
    mis_log::instance()->set_config(mis_config);

    // Read input file
    std::vector<int> rv(g.next_free_id);
    for (int i = 0; i < g.nodes.size(); i++) {
        int v = g.nodes[i];
        rv[v] = i;
    }
    
    std::vector<std::vector<int>> graph(g.nodeCount());
    for (int i = 0; i < g.nodes.size(); i++) {
        int v = g.nodes[i];
        for (auto [w, status] : g.adj[v]) {
            DS_ASSERT(status == FORCED);
            int j = rv[w];
            graph[i].push_back(j);
        }
    }

    mis_log::instance()->number_of_nodes = graph.size();
    unsigned int num_edges = 0;
    for (auto &v : graph) num_edges += v.size();
    mis_log::instance()->number_of_edges = num_edges;

    timer t;
    t.restart();
    std::vector<bool> MIS;
    MIS = getExactMISCombined(graph, mis_config);

    std::vector<int> ds;
    for (int i = 0; i < MIS.size(); i++) {
        if (MIS[i]) ds.push_back(g.nodes[i]);
    }
    return ds;
}
}  // namespace DSHunter