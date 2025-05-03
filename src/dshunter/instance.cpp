#include "instance.h"

#include <queue>
#include <set>

#include "utils.h"

namespace DSHunter {

Node::Node(int v, bool is_extra) : n_closed({ v }),
                                   domination_status(DominationStatus::UNDOMINATED),
                                   membership_status(MembershipStatus::UNDECIDED),
                                   is_extra(is_extra) {}

Instance::Instance() = default;
// Constructs graph from input stream assuming DIMACS-like .gr format.
Instance::Instance(std::istream &in) : ds{} {
    std::string line;
    int header_edges, read_edges = 0;
    while (std::getline(in, line)) {
        std::stringstream tokens(line);
        std::string s;
        tokens >> s;
        if (s[0] == 'c')
            continue;
        if (s[0] == 'p') {
            parse_header(tokens, header_edges);
        } else {
            int a = stoi(s);
            int b = 0;
            tokens >> b;
            ++read_edges;
            unorderedAddDirectedEdge(a, b);
            unorderedAddDirectedEdge(b, a);
        }
    }

    sortAdjacencyLists();

    if (header_edges != read_edges)
        throw std::logic_error("expected " + std::to_string(header_edges) + " edges, found " +
                               std::to_string(read_edges));
}

void Instance::parse_header(std::stringstream &tokens, int &header_edges) {
    std::string problem;
    int n_nodes = 0;
    tokens >> problem >> n_nodes >> header_edges;

    all_nodes.reserve(n_nodes + 1);
    all_nodes.emplace_back(0, false);
    all_nodes[0].n_closed = {};
    for (int i = 1; i <= n_nodes; ++i) {
        nodes.push_back(i);
        all_nodes.emplace_back(i, false);
    }
}

// Returns the number of nodes in the graph.
size_t Instance::nodeCount() const { return nodes.size(); }
size_t Instance::disregardedNodeCount() const {
    size_t cnt = 0;
    for (auto v : nodes)
        if (isDisregarded(v)) cnt++;
    return cnt;
}

bool Instance::isDominated(int v) const {
    return all_nodes[v].domination_status == DominationStatus::DOMINATED;
}

void Instance::markDominated(int v) {
    DS_TRACE(std::cerr << __func__ << dbg(v) << std::endl);
    all_nodes[v].domination_status = DominationStatus::DOMINATED;
}

bool Instance::isTaken(int v) const {
    return all_nodes[v].membership_status == MembershipStatus::TAKEN;
}

void Instance::markTaken(int v) {
    all_nodes[v].domination_status = DominationStatus::DOMINATED;
    all_nodes[v].membership_status = MembershipStatus::TAKEN;
}

bool Instance::isDisregarded(int v) const {
    return all_nodes[v].membership_status == MembershipStatus::DISREGARDED;
}

void Instance::markDisregarded(int v) {
    DS_TRACE(std::cerr << __func__ << dbg(v) << std::endl);
    all_nodes[v].membership_status = MembershipStatus::DISREGARDED;
}

void Instance::forceEdge(int u, int v) {
    DS_TRACE(std::cerr << __func__ << dbg(u) << dbg(v) << std::endl);
    DS_ASSERT(hasEdge(u, v));
    DS_ASSERT(getEdgeStatus(u, v) != EdgeStatus::FORCED);
    setEdgeStatus(u, v, EdgeStatus::FORCED);
    markDominated(u);
    markDominated(v);

    // All vertices that see both endpoints of this edge must be dominated by one of them,
    // so we can mark them as dominated.
    auto edgeNeighbourhood = intersect(all_nodes[u].n_open, all_nodes[v].n_open);
    for (auto w : edgeNeighbourhood) {
        markDominated(w);
    }
}

EdgeStatus Instance::getEdgeStatus(int u, int v) const {
    auto it = lower_bound(all_nodes[u].adj.begin(), all_nodes[u].adj.end(), Endpoint{ v, EdgeStatus::ANY });
    DS_ASSERT(it != all_nodes[u].adj.end());
    return it->status;
}

// Returns the degree of given node.
int Instance::deg(int v) const { return (int)all_nodes[v].adj.size(); }

int Instance::forcedDeg(int v) const {
    int res = 0;
    for (auto e : all_nodes[v].adj)
        if (e.status == EdgeStatus::FORCED)
            res++;
    return res;
}

// Creates and returns the id of the created node.
// Complexity: O(1)
int Instance::addNode() {
    DS_TRACE(std::cerr << __func__ << std::endl);
    int v = all_nodes.size();
    nodes.push_back(v);
    all_nodes.emplace_back(v);
    return v;
}

bool Instance::hasNode(int v) {
    return (int)all_nodes.size() > v && !all_nodes[v].n_closed.empty();
}

// Removes the node with given id.
// Complexity: O(deg(v) + sum over deg(v) of neighbours)
void Instance::removeNode(int v) {
    DS_TRACE(std::cerr << __func__ << dbg(v) << std::endl);
    if (!hasNode(v))
        return;
    for (auto [u, status] : all_nodes[v].adj) {
        // Edges like this can only be removed by calling take().
        DS_ASSERT(status != EdgeStatus::FORCED || isTaken(v));
        removeDirectedEdge(u, v);
    }

    all_nodes[v].adj.clear();
    all_nodes[v].n_open.clear();
    all_nodes[v].n_closed.clear();
    remove(nodes, v);
}

// Removes nodes in the given list from the graph.
// Complexity: O(sum of deg(v) over l ∪ N(l))
void Instance::removeNodes(const std::vector<int> &l) {
    for (auto &v : l) removeNode(v);
}

// Adds an unconstrained edge between nodes with id's u and v.
// Complexity: O(deg(v)), due to maintaining adjacency list to be sorted.
void Instance::addEdge(int u, int v, EdgeStatus status) {
    DS_TRACE(std::cerr << __func__ << dbg(u) << dbg(v) << std::endl);
    addDirectedEdge(u, v, EdgeStatus::UNCONSTRAINED);
    addDirectedEdge(v, u, EdgeStatus::UNCONSTRAINED);
    if (status == EdgeStatus::FORCED) {
        forceEdge(u, v);
    }
}

// Removes edge (v, w) from the graph.
// Complexity: O(deg(v) + deg(w))
void Instance::removeEdge(int u, int v) {
    DS_TRACE(std::cerr << __func__ << dbg(u) << dbg(v) << std::endl);
    removeDirectedEdge(u, v);
    removeDirectedEdge(v, u);
}

// Returns the number of edges in the graph.
// Complexity: O(n)
int Instance::edgeCount() const {
    int sum_deg = 0;
    for (auto i : nodes) sum_deg += deg(i);
    return sum_deg / 2;
}

int Instance::forcedEdgeCount() const {
    int sum_deg = 0;
    for (auto i : nodes) sum_deg += forcedDeg(i);
    return sum_deg / 2;
}

// Returns true if and only if undirected edge (u, v) is present in the graph.
// Complexity: O(log(deg(u))) !
bool Instance::hasEdge(int u, int v) const {
    auto &node = all_nodes[u];
    return std::binary_search(node.n_open.begin(), node.n_open.end(), v);
}

// Inserts given node to the dominating set, changing the status of
// it's neighours to DOMINATED if they are not, the node is removed from the graph afterwards.
// Complexity: O(deg(v)) or O(sum of degrees of neighbours) in case of extra vertices.
void Instance::take(int v) {
    DS_TRACE(std::cerr << __func__ << dbg(v) << std::endl);
    DS_ASSERT(!isTaken(v));

    auto &node = all_nodes[v];
    if (node.is_extra) {
        // Copy as we will invalidate our iterator if we operate on the original vector.
        auto n_open = node.n_open;
        for (auto u : n_open) {
            DS_ASSERT(!isTaken(u));
            take(u);
        }

        return;
    }
    node.membership_status = MembershipStatus::TAKEN;

    ds.push_back(v);
    for (auto u : node.n_open) {
        DS_ASSERT(!isTaken(u));
        markDominated(u);
    }

    removeNode(v);
}

// Splits the list of graph nodes into individual connected components.
// Complexity: O(n + m)
std::vector<std::vector<int>> Instance::split() const {
    std::vector<int> component(all_nodes.size(), -1);
    int components = 0;

    // Assign nodes to connected components using breadth-first search.
    for (auto v : nodes) {
        if (component[v] < 0) {
            component[v] = components;

            std::queue<int> q;
            q.push(v);
            while (!q.empty()) {
                int w = q.front();
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

    std::vector<std::vector<int>> result(components, std::vector<int>());
    for (auto v : nodes) {
        result[component[v]].push_back(v);
    }
    return result;
}

void Instance::setEdgeStatus(int u, int v, EdgeStatus status) {
    auto it_u = lower_bound(all_nodes[u].adj.begin(), all_nodes[u].adj.end(), Endpoint{ v, EdgeStatus::ANY });
    DS_ASSERT(it_u != all_nodes[u].adj.end());
    auto it_v = lower_bound(all_nodes[v].adj.begin(), all_nodes[v].adj.end(), Endpoint{ u, EdgeStatus::ANY });
    DS_ASSERT(it_v != all_nodes[v].adj.end());

    it_u->status = it_v->status = status;
}

void Instance::addDirectedEdge(int u, int v, EdgeStatus status) {
    auto &node = all_nodes[u];
    insert(node.adj, Endpoint{ v, status });
    insert(node.n_open, v);
    insert(node.n_closed, v);
}

void Instance::removeDirectedEdge(int u, int v) {
    auto &node = all_nodes[u];
    remove(node.adj, Endpoint{ v, EdgeStatus::ANY });
    remove(node.n_open, v);
    remove(node.n_closed, v);
}

void Instance::unorderedAddDirectedEdge(int u, int v, EdgeStatus status) {
    auto &node = all_nodes[u];
    node.adj.emplace_back(v, status);
    node.n_open.push_back(v);
    node.n_closed.push_back(v);
}

void Instance::sortAdjacencyLists() {
    for (auto &node : all_nodes) {
        sort(node.adj.begin(), node.adj.end());
        sort(node.n_open.begin(), node.n_open.end());
        sort(node.n_closed.begin(), node.n_closed.end());
    }
}

const Node &Instance::operator[](int v) const { return all_nodes[v]; }

}  // namespace DSHunter