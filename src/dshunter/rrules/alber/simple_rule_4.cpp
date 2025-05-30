#include "../rrules.h"
namespace {

bool tryMidpoint(DSHunter::Instance& g, const bool forced_by_edge, const int u, const int v, const int w) {
    if (!g.isDisregarded(u) && g.hasEdge(u, v) && g.hasEdge(u, w)) {
        if (forced_by_edge) {
            DS_TRACE(std::cerr << "applying " << __func__ << "(take)" << dbg(u) << std::endl);
            g.take(u);
        }
        return true;
    }

    return false;
}

}  // namespace

namespace DSHunter {

bool alberSimpleRule4(Instance& g) {
    const auto nodes = g.nodes;
    bool reduced = false;

    for (const auto v : nodes) {
        if (g.hasNode(v) && g.isDominated(v) && g.deg(v) == 3) {
            auto [u_1, s_1] = g[v].adj[0];
            auto [u_2, s_2] = g[v].adj[1];
            auto [u_3, s_3] = g[v].adj[2];

            const int n_forced_edges = static_cast<int>(s_1) + static_cast<int>(s_2) + static_cast<int>(s_3);
            // There can be at most one forced edge, and it needs to lead to a vertex that can
            // dominate all three others.
            const bool possibly_valid = !g.isDominated(u_1) && !g.isDominated(u_2) &&
                                  !g.isDominated(u_3) && n_forced_edges <= 1;

            if (possibly_valid) {
                if (tryMidpoint(g, static_cast<bool>(s_1), u_1, u_2, u_3) ||
                    tryMidpoint(g, static_cast<bool>(s_2), u_2, u_1, u_3) ||
                    tryMidpoint(g, static_cast<bool>(s_3), u_3, u_1, u_2)) {
                    DS_TRACE(std::cerr << "applying " << __func__ << dbg(v) << std::endl);
                    g.removeNode(v);
                    reduced = true;
                }
            }
        }
    }

    return reduced;
}

ReductionRule AlberSimpleRule4("AlberSimpleRule4 (dominated degree 3 vertex removal)", alberSimpleRule4, 2, 1);
}  // namespace DSHunter