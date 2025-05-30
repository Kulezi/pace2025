#include "instance.h"

#include <queue>
#include <set>
#include <sstream>
#include <ranges>

#include "utils.h"

namespace DSHunter {
using std::logic_error;
using std::string, std::getline, std::stringstream, std::istream, std::to_string, std::ranges::sort, std::ranges::binary_search;
using std::vector;

Node::Node() : domination_status(DominationStatus::DOMINATED), membership_status(MembershipStatus::DISREGARDED), is_extra(false) {}

Node::Node(int v, const bool is_extra) : n_closed({ v }), dominators({ v }), dominatees({ v }), domination_status(DominationStatus::UNDOMINATED), membership_status(MembershipStatus::UNDECIDED), is_extra(is_extra) {}

Instance::Instance() = default;

Instance::Instance(istream &in) {
    string line;
    int header_edges;

    string problem;
    while (getline(in, line)) {
        stringstream tokens(line);
        string s;
        tokens >> s;
        if (s[0] == 'c')
            continue;
        if (s[0] == 'p') {
            int n_nodes = 0;
            tokens >> problem >> n_nodes >> header_edges;
            all_nodes.reserve(n_nodes + 1);

            // Dummy node for 1-indexing.
            all_nodes.emplace_back();

            if (problem == "ads") {
                int d;
                tokens >> d;
                parseADS(in, n_nodes, header_edges, d);
            } else {
                for (int i = 1; i <= n_nodes; ++i) {
                    nodes.push_back(i);
                    all_nodes.emplace_back(i, false);
                }
                parseDS(in, n_nodes, header_edges);
            }

            break;
        }
    }

    sortAdjacencyLists();
}

void Instance::parseADS(istream &in, int n_nodes, int header_edges, int d) {
    // Read dominating set elements.
    {
        string line;
        getline(in, line);
        stringstream tokens(line);
        ds = vector<int>(d);
        for (auto &v : ds) tokens >> v;
    }

    // Read node descriptions.
    {
        for (int i = 1; i <= n_nodes; i++) {
            string line;
            getline(in, line);
            stringstream tokens(line);

            int v, s_d, s_m, e;
            tokens >> v >> s_d >> s_m >> e;

            while (static_cast<int>(all_nodes.size()) <= v) {
                all_nodes.emplace_back(all_nodes.size(), false);
            }

            nodes.push_back(v);
            all_nodes[v].membership_status = static_cast<MembershipStatus>(s_m);
            all_nodes[v].domination_status = static_cast<DominationStatus>(s_d);
            if (all_nodes[v].membership_status == MembershipStatus::DISREGARDED || all_nodes[v].domination_status == DominationStatus::DOMINATED) {
                all_nodes[v].dominatees = {};
                all_nodes[v].dominators = {};
            }
            all_nodes[v].is_extra = static_cast<bool>(e);
        }
    }

    for (int i = 1; i <= header_edges; i++) {
        string line;
        getline(in, line);
        stringstream tokens(line);

        string s;
        int a, b, f;
        tokens >> a >> b >> f;
        initAddEdge(a, b, f ? EdgeStatus::FORCED : EdgeStatus::UNCONSTRAINED);
    }
}

void Instance::parseDS(istream &in, const int n_nodes, const int header_edges) {
    int read_edges = 0;
    string line;
    while (getline(in, line)) {
        stringstream tokens(line);
        string s;
        tokens >> s;
        if (s[0] == 'c')
            continue;
        const int a = stoi(s);
        int b = 0;
        tokens >> b;
        DS_ASSERT(a > 0 && a <= n_nodes);
        DS_ASSERT(b > 0 && b <= n_nodes);

        ++read_edges;
        initAddEdge(a, b);
    }

    if (header_edges != read_edges)
        throw logic_error("expected " + to_string(header_edges) + " edges, found " +
                          to_string(read_edges));
}

int Instance::nodeCount() const { return static_cast<int>(nodes.size()); }
int Instance::disregardedNodeCount() const {
    int cnt = 0;
    for (const auto v : nodes)
        if (isDisregarded(v))
            cnt++;
    return cnt;
}

bool Instance::isDominated(const int v) const {
    return all_nodes[v].domination_status == DominationStatus::DOMINATED;
}

void Instance::markDominated(const int v) {
    DS_TRACE(std::cerr << __func__ << dbg(v) << std::endl);
    auto &node = all_nodes[v];
    node.domination_status = DominationStatus::DOMINATED;
    for (const auto u : node.dominators) {
        remove(all_nodes[u].dominatees, v);
    }
    node.dominators = {};
}

bool Instance::isTaken(const int v) const {
    return all_nodes[v].membership_status == MembershipStatus::TAKEN;
}

void Instance::markTaken(const int v) {
    DS_ASSERT(!isTaken(v));
    markDominated(v);
    all_nodes[v].membership_status = MembershipStatus::TAKEN;
}

bool Instance::isDisregarded(const int v) const {
    return all_nodes[v].membership_status == MembershipStatus::DISREGARDED;
}

void Instance::markDisregarded(const int v) {
    DS_TRACE(std::cerr << __func__ << dbg(v) << std::endl);
    DS_ASSERT(!isDisregarded(v));
    auto &node = all_nodes[v];
    node.membership_status = MembershipStatus::DISREGARDED;
    for (auto u : node.dominatees) {
        remove(all_nodes[u].dominators, v);
    }
    node.dominatees = {};
}

void Instance::ignore(const int v) {
    if (!hasNode(v))
        return;

    vector<int> to_take;
    for (auto [u, status] : all_nodes[v].adj) {
        // Edges like this can only be removed by calling take().
        if (status == EdgeStatus::FORCED && !isTaken(v)) {
            to_take.push_back(u);
        }
        removeDirectedEdge(u, v);
    }

    all_nodes[v] = Node();
    remove(nodes, v);
    for (const auto u : to_take) take(u);
}

void Instance::forceEdge(const int u, const int v) {
    DS_TRACE(std::cerr << __func__ << dbg(u) << dbg(v) << std::endl);
    DS_ASSERT(hasEdge(u, v));
    DS_ASSERT(getEdgeStatus(u, v) != EdgeStatus::FORCED);
    setEdgeStatus(u, v, EdgeStatus::FORCED);
    markDominated(u);
    markDominated(v);

    // All vertices that see both endpoints of this edge must be dominated by one of them,
    // so we can mark them as dominated.
    for (const auto w : intersect(all_nodes[u].dominatees, all_nodes[v].dominatees)) {
        markDominated(w);
    }
}

EdgeStatus Instance::getEdgeStatus(const int u, const int v) const {
    auto it = lower_bound(all_nodes[u].adj.begin(), all_nodes[u].adj.end(), Endpoint{ v, EdgeStatus::ANY });
    DS_ASSERT(it != all_nodes[u].adj.end());
    return it->status;
}

int Instance::deg(const int v) const { return static_cast<int>(all_nodes[v].adj.size()); }

int Instance::forcedDeg(const int v) const {
    int res = 0;
    for (auto e : all_nodes[v].adj)
        if (e.status == EdgeStatus::FORCED)
            res++;
    return res;
}

int Instance::addNode() {
    DS_TRACE(std::cerr << __func__ << std::endl);
    int v = static_cast<int>(all_nodes.size());
    nodes.push_back(v);
    all_nodes.emplace_back(v);
    return v;
}

bool Instance::hasNode(const int v) const {
    return static_cast<int>(all_nodes.size()) > v && !all_nodes[v].n_closed.empty();
}

void Instance::removeNode(const int v) {
    DS_TRACE(std::cerr << __func__ << dbg(v) << std::endl);
    if (!hasNode(v))
        return;
    for (auto [u, status] : all_nodes[v].adj) {
        // Edges like this can only be removed by calling take().
        DS_ASSERT(status != EdgeStatus::FORCED || isTaken(v));
        removeDirectedEdge(u, v);
    }

    all_nodes[v] = Node();
    remove(nodes, v);
}

void Instance::removeNodes(const vector<int> &l) {
    for (auto &v : l) removeNode(v);
}

void Instance::addEdge(const int u, const int v, const EdgeStatus status) {
    DS_TRACE(std::cerr << __func__ << dbg(u) << dbg(v) << std::endl);
    addDirectedEdge(u, v);
    addDirectedEdge(v, u);
    if (status == EdgeStatus::FORCED) {
        forceEdge(u, v);
    }
}

void Instance::removeEdge(const int u, const int v) {
    DS_TRACE(std::cerr << __func__ << dbg(u) << dbg(v) << std::endl);
    removeDirectedEdge(u, v);
    removeDirectedEdge(v, u);
}

int Instance::edgeCount() const {
    int sum_deg = 0;
    for (const auto i : nodes) sum_deg += deg(i);
    return sum_deg / 2;
}

int Instance::forcedEdgeCount() const {
    int sum_deg = 0;
    for (const auto i : nodes) sum_deg += forcedDeg(i);
    return sum_deg / 2;
}

bool Instance::hasEdge(const int u, const int v) const {
    auto &node = all_nodes[u];
    return binary_search(node.n_open, v);
}

void Instance::take(const int v) {
    DS_TRACE(std::cerr << __func__ << dbg(v) << std::endl);
    DS_ASSERT(!isTaken(v));
    DS_ASSERT(!isDisregarded(v));

    auto &node = all_nodes[v];
    if (node.is_extra) {
        // Copy as we will invalidate our iterator if we operate on the original vector.
        for (const auto n_open = node.n_open; const auto u : n_open) {
            DS_ASSERT(!isTaken(u));
            take(u);
        }

        return;
    }
    node.membership_status = MembershipStatus::TAKEN;

    ds.push_back(v);
    for (const auto dominatees = node.dominatees; const auto u : dominatees) {
        markDominated(u);
    }

    removeNode(v);
}

vector<vector<int>> Instance::split() const {
    vector component(all_nodes.size(), -1);
    int components = 0;

    // Assign nodes to connected components using breadth-first search.
    for (auto v : nodes) {
        if (component[v] < 0) {
            component[v] = components;

            std::queue<int> q;
            q.push(v);
            while (!q.empty()) {
                const int w = q.front();
                q.pop();

                for (auto u : all_nodes[w].n_open) {
                    if (component[u] < 0) {
                        component[u] = components;
                        q.push(u);
                    }
                }
            }

            ++components;
        }
    }

    vector result(components, vector<int>());
    for (auto v : nodes) {
        result[component[v]].push_back(v);
    }
    return result;
}

bool Instance::isSolvable() const {
    return std::ranges::none_of(nodes, [&](int v) {
        return !isDominated(v) && all_nodes[v].dominators.empty();
    });
}

void Instance::setEdgeStatus(const int u, const int v, const EdgeStatus status) {
    const auto it_u = lower_bound(all_nodes[u].adj.begin(), all_nodes[u].adj.end(), Endpoint{ v, EdgeStatus::ANY });
    DS_ASSERT(it_u != all_nodes[u].adj.end());
    const auto it_v = lower_bound(all_nodes[v].adj.begin(), all_nodes[v].adj.end(), Endpoint{ u, EdgeStatus::ANY });
    DS_ASSERT(it_v != all_nodes[v].adj.end());

    it_u->status = it_v->status = status;
}

void Instance::addDirectedEdge(const int u, const int v) {
    auto &node = all_nodes[u];
    insert(node.adj, Endpoint{ v, EdgeStatus::UNCONSTRAINED });
    insert(node.n_open, v);
    insert(node.n_closed, v);
    if (!isDominated(u))
        insert(node.dominators, v);
    if (!isDisregarded(u))
        insert(node.dominatees, v);
}

void Instance::removeDirectedEdge(const int u, const int v) {
    auto &node = all_nodes[u];
    remove(node.adj, Endpoint{ v, EdgeStatus::ANY });
    remove(node.n_open, v);
    remove(node.n_closed, v);
    remove(node.dominators, v);
    remove(node.dominatees, v);
}

void Instance::initAddEdge(const int u, const int v, const EdgeStatus status) {
    initAddDirectedEdge(u, v, status);
    initAddDirectedEdge(v, u, status);
}

void Instance::initAddDirectedEdge(const int u, int const v, const EdgeStatus status) {
    auto &node = all_nodes[u];
    node.adj.emplace_back(v, status);
    node.n_open.push_back(v);
    node.n_closed.push_back(v);
    if (!isDominated(u))
        node.dominators.push_back(v);
    if (!isDisregarded(u))
        node.dominatees.push_back(v);
}

void Instance::sortAdjacencyLists() {
    for (auto &node : all_nodes) {
        std::sort(node.adj.begin(), node.adj.end());
        sort(node.n_open);
        sort(node.n_closed);
        sort(node.dominators);
        sort(node.dominatees);
    }
}

const Node &Instance::operator[](const int v) const { return all_nodes[v]; }

void Instance::exportADS(std::ostream &output) {
    output << "p ads " << nodeCount() << " " << edgeCount() << " " << ds.size() << "\n";
    for (const auto v : ds) output << v << " ";
    output << "\n";

    for (const auto v : nodes) {
        output << v << " " << static_cast<int>(all_nodes[v].domination_status) << " " << static_cast<int>(all_nodes[v].membership_status) << " " << all_nodes[v].is_extra << "\n";
    }

    for (const auto u : nodes) {
        for (const auto [v, status] : all_nodes[u].adj) {
            if (u < v)
                output << u << " " << v << " " << static_cast<int>(status) << "\n";
        }
    }
}

}  // namespace DSHunter