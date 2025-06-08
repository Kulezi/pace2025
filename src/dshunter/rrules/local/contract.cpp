#include "../rrules.h"
#include<iostream>
namespace {
using namespace DSHunter;
using std::vector;

bool contract(Instance &g, int u) {
    if (g.deg(u) != 2 || g.forcedDeg(u) != 0) return false;
    int x = g[u].n_open[0];
    int v = g[u].n_open[1];
    if (g.deg(v) != 2 || g.forcedDeg(v) != 0) std::swap(x, v);
    if (g.deg(v) != 2 || g.forcedDeg(v) != 0) return false;

    int y = g[v].n_open[0];
    if (y == u) y = g[v].n_open[1];

    if (g.isDisregarded(u) || g.isDisregarded(v) || g.isDisregarded(x) || g.isDisregarded(y)) return false;
    if (g.isDominated(u) || g.isDominated(v) || g.isDominated(x) || g.isDominated(y)) return false;

    if (x == y) {
        g.take(x);
        g.removeNode(u);
        g.removeNode(v);
        return true;
    }

    g.contract(x, u);
    g.contract(x, v);
    g.contract(x, y);
    return true;
}
}  // namespace

namespace DSHunter {
bool contractRule(Instance &g) {
    bool reduced = false;
    const auto nodes = g.nodes;
    for (auto u : nodes) {
        if (g.hasNode(u) && contract(g, u))
            reduced = true;
    }

    return reduced;
}

ReductionRule ContractRule("contractRule", contractRule, 1, 1);

}  // namespace DSHunter
