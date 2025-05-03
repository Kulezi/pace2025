#include "../rrules.h"
namespace {

bool haveCommonNonDisregardedNeighbour(const DSHunter::Instance& g, int u, int v_1, int v_2) {
    for (auto w : g[v_1].n_open) {
        if (w == u || g.isDisregarded(w))
            continue;
        if (g.hasEdge(w, v_2))
            return true;
    }
    return false;
}

}  // namespace

namespace DSHunter {

bool alberSimpleRule3(Instance& g) {
    auto nodes = g.nodes;
    bool reduced = false;

    for (auto v : nodes) {
        if (g.hasNode(v) && g.isDominated(v) && g.deg(v) == 2) {
            auto [u_1, s_1] = g[v].adj.front();
            auto [u_2, s_2] = g[v].adj[1];

            // In this case it actually might be optimal to take this vertex instead of the two.
            if (s_1 == EdgeStatus::FORCED && s_2 == EdgeStatus::FORCED)
                continue;

            bool should_remove = !g.isDominated(u_1) && !g.isDominated(u_2) &&
                                 (g.hasEdge(u_1, u_2) || haveCommonNonDisregardedNeighbour(g, v, u_1, u_2));

            if (should_remove) {
                DS_TRACE(std::cerr << "applying " << __func__ << dbg(v) << std::endl);

                DS_ASSERT(s_1 != EdgeStatus::FORCED || s_2 != EdgeStatus::FORCED);
                if (s_1 == EdgeStatus::FORCED)
                    g.take(u_1);
                if (s_2 == EdgeStatus::FORCED)
                    g.take(u_2);

                g.removeNode(v);

                reduced = true;
            }
        }
    }

    return reduced;
}

ReductionRule AlberSimpleRule3("AlberSimpleRule3 (dominated degree 2 vertex removal)", alberSimpleRule3, 2, 1);

}  // namespace DSHunter