#include "../rrules.h"
namespace {

bool haveCommonNeighbour(const DSHunter::Instance& g, int u, int v) {
    for (auto [w, _] : g[u].adj) {
        if (v == w)
            continue;
        if (g.hasEdge(v, v))
            return true;
    }
    return false;
}

}  // namespace

namespace DSHunter {

bool alberSimpleRule3(Instance& g) {
    for (auto v : g.nodes) {
        if (g.isDominated(v) && g.deg(v) == 2) {
            auto [u_1, s_1] = g[v].adj.front();
            auto [u_2, s_2] = g[v].adj[1];
            // In this case it actually might be optimal to take this vertex instead of the two.
            if (s_1 == EdgeStatus::FORCED && s_2 == EdgeStatus::FORCED)
                continue;
            bool should_remove = !g.isDominated(u_1) && !g.isDominated(u_2) &&
                                 (g.hasEdge(u_1, u_2) || haveCommonNeighbour(g, u_1, u_2));

            if (should_remove) {
                DS_TRACE(std::cerr << "applying " << __func__ << dbg(v) << std::endl);

                DS_ASSERT(s_1 != EdgeStatus::FORCED || s_2 != EdgeStatus::FORCED);
                if (s_1 == EdgeStatus::FORCED)
                    g.take(u_1);
                if (s_2 == EdgeStatus::FORCED)
                    g.take(u_2);

                g.removeNode(v);

                return true;
            }
        }
    }

    return false;
}

ReductionRule AlberSimpleRule3("AlberSimpleRule3 (dominated degree 2 vertex removal)", alberSimpleRule3, 2, 1);

}  // namespace DSHunter