#include "instance.h"

#include <queue>
#include <set>

#include "utils.h"

namespace DSHunter {

Node::Node() : domination_status(DominationStatus::DOMINATED), membership_status(MembershipStatus::DISREGARDED), is_extra(false) {}
Node::Node(int v, bool is_extra) : n_closed({ v }), dominators({ v }), dominatees({ v }), domination_status(DominationStatus::UNDOMINATED), membership_status(MembershipStatus::UNDECIDED), is_extra(is_extra) {}

Instance::Instance() = default;

// Constructs a graph from the input stream assuming DIMACS-like .gr format.
Instance::Instance(std::istream &in) {
    std::string line;
    int header_edges;

    std::string problem;
    while (std::getline(in, line)) {
        std::stringstream tokens(line);
        std::string s;
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

void Instance::parseADS(std::istream &in, int n_nodes, int header_edges, int d) {
    // Read dominating set elements.
    {
        std::string line;
        std::getline(in, line);
        std::stringstream tokens(line);
        ds = std::vector<int>(d);
        for (auto &v : ds) tokens >> v;
    }

    // Read node descriptions.
    {
        for (int i = 1; i <= n_nodes; i++) {
            std::string line;
            std::getline(in, line);
            std::cerr << line << std::endl;
            std::stringstream tokens(line);

            int v, d, m, e;
            tokens >> v >> d >> m >> e;

<<<<<<< HEAD
            while (static_cast<int>(all_nodes.size()) <= v) {
=======
            while ((int)all_nodes.size() <= v) {
>>>>>>> 3fa243b (instance.h: add exporting in .ads format)
                all_nodes.emplace_back(all_nodes.size(), false);
            }

            nodes.push_back(v);
            all_nodes[v].membership_status = static_cast<MembershipStatus>(m);
            all_nodes[v].domination_status = static_cast<DominationStatus>(d);
            all_nodes[v].is_extra = static_cast<bool>(e);
        }
    }

    for (int i = 1; i <= header_edges; i++) {
        std::string line;
        std::getline(in, line);
        std::stringstream tokens(line);

        std::string s;
        int a, b, f;
        tokens >> a >> b >> f;
        initAddEdge(a, b, f ? EdgeStatus::FORCED : EdgeStatus::UNCONSTRAINED);
    }
}

void Instance::parseDS(std::istream &in, int n_nodes, int header_edges) {
    int read_edges = 0;
    std::string line;
    while (std::getline(in, line)) {
        std::stringstream tokens(line);
        std::string s;
        tokens >> s;
        if (s[0] == 'c')
            continue;
        int a = stoi(s);
        int b = 0;
        tokens >> b;
        DS_ASSERT(a > 0 && a <= n_nodes);
        DS_ASSERT(b > 0 && b <= n_nodes);

        ++read_edges;
        initAddEdge(a, b);
    }

    if (header_edges != read_edges)
        throw std::logic_error("expected " + std::to_string(header_edges) + " edges, found " +
                               std::to_string(read_edges));
}

// Returns the number of nodes in the graph.
size_t Instance::nodeCount() const { return nodes.size(); }
size_t Instance::disregardedNodeCount() const {
    size_t cnt = 0;
    for (auto v : nodes)
        if (isDisregarded(v))
            cnt++;
    return cnt;
}

bool Instance::isDominated(int v) const {
    return all_nodes[v].domination_status == DominationStatus::DOMINATED;
}

void Instance::markDominated(int v) {
    DS_TRACE(std::cerr << __func__ << dbg(v) << std::endl);
    auto &node = all_nodes[v];
    node.domination_status = DominationStatus::DOMINATED;
    for (auto u : node.dominators) {
        remove(all_nodes[u].dominatees, v);
    }
    node.dominators = {};
}

bool Instance::isTaken(int v) const {
    return all_nodes[v].membership_status == MembershipStatus::TAKEN;
}

void Instance::markTaken(int v) {
    DS_ASSERT(!isTaken(v));
    markDominated(v);
    all_nodes[v].membership_status = MembershipStatus::TAKEN;
}

bool Instance::isDisregarded(int v) const {
    return all_nodes[v].membership_status == MembershipStatus::DISREGARDED;
}

void Instance::markDisregarded(int v) {
    DS_TRACE(std::cerr << __func__ << dbg(v) << std::endl);
    DS_ASSERT(!isDisregarded(v));
    auto &node = all_nodes[v];
    node.membership_status = MembershipStatus::DISREGARDED;
    for (auto u : node.dominatees) {
        remove(all_nodes[u].dominators, v);
    }
    node.dominatees = {};
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
    for (auto w : intersect(all_nodes[u].dominatees, all_nodes[v].dominatees)) {
        markDominated(w);
    }
}

EdgeStatus Instance::getEdgeStatus(int u, int v) const {
    auto it = lower_bound(all_nodes[u].adj.begin(), all_nodes[u].adj.end(), Endpoint{ v, EdgeStatus::ANY });
    DS_ASSERT(it != all_nodes[u].adj.end());
    return it->status;
}

// Returns the degree of a given node.
int Instance::deg(int v) const { return static_cast<int>(all_nodes[v].adj.size()); }

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
    int v = static_cast<int>(all_nodes.size());
    nodes.push_back(v);
    all_nodes.emplace_back(v);
    return v;
}

bool Instance::hasNode(int v) const {
    return static_cast<int>(all_nodes.size()) > v && !all_nodes[v].n_closed.empty();
}

// Removes the node with a given id.
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

    all_nodes[v] = Node();
    remove(nodes, v);
}

// Removes nodes in the given list from the graph.
// Complexity: O(sum of deg(v) over l ∪ N(l))
void Instance::removeNodes(const std::vector<int> &l) {
    for (auto &v : l) removeNode(v);
}

// Adds an unconstrained edge between nodes with id's u and v.
// Complexity: O(deg(v)), due to maintaining the adjacency list to be sorted.
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

// Returns true if and only if the undirected edge (u, v) is present in the graph.
// Complexity: O(log(deg(u))) !
bool Instance::hasEdge(int u, int v) const {
    auto &node = all_nodes[u];
    return std::ranges::binary_search(node.n_open, v);
}

// Inserts a given node to the dominating set, changing the status of
// its neighbors to DOMINATED if they are not, the node is removed from the graph afterward.
// Complexity: O(deg(v)) or O(sum of degrees of neighbours) in case of extra vertices.
void Instance::take(int v) {
    DS_TRACE(std::cerr << __func__ << dbg(v) << std::endl);
    DS_ASSERT(!isTaken(v));
    DS_ASSERT(!isDisregarded(v));

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
    auto dominatees = node.dominatees;
    for (auto u : dominatees) {
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

    std::vector result(components, std::vector<int>());
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
    if (!isDominated(u)) insert(node.dominators, v);
    if (!isDisregarded(u)) insert(node.dominatees, v);
}

void Instance::removeDirectedEdge(int u, int v) {
    auto &node = all_nodes[u];
    remove(node.adj, Endpoint{ v, EdgeStatus::ANY });
    remove(node.n_open, v);
    remove(node.n_closed, v);
    remove(node.dominators, v);
}

void Instance::initAddEdge(int u, int v, EdgeStatus status) {
    initAddDirectedEdge(u, v, status);
    initAddDirectedEdge(v, u, status);
}

void Instance::initAddDirectedEdge(int u, int v, EdgeStatus status) {
    auto &node = all_nodes[u];
    node.adj.emplace_back(v, status);
    node.n_open.push_back(v);
    node.n_closed.push_back(v);
    if (!isDominated(u)) node.dominators.push_back(v);
    if (!isDisregarded(u)) node.dominatees.push_back(v);
}

void Instance::sortAdjacencyLists() {
    for (auto &node : all_nodes) {
        sort(node.adj.begin(), node.adj.end());
        std::ranges::sort(node.n_open);
        std::ranges::sort(node.n_closed);
        std::ranges::sort(node.dominators);
        std::ranges::sort(node.dominatees);
    }
}

const Node &Instance::operator[](int v) const { return all_nodes[v]; }

// .ads format description:
//  first line is 'p ads n m d' where:
//      n is the number of remaining nodes
//      m is the number of remaining edges
//      d is the number of nodes already known to be in the optimal dominating set
//      (those nodes are guaranteed to not be present in the graph)
//  second line is v_1, v_2, ..., v_d, being the list of nodes already known to be in the optimal
//  dominating set following it are n lines describing the nodes of the remaining graph in format 'v
//  s_d s_m e', where:
//      v is the node number in the original graph, note nodes may not be numbered from 1 to n.
//      s_d is the nodes domination status, 0 means undominated, 1 means dominated
//      s_m is the nodes solution membership status, 0 means maybe, 1 means no, 2 means yes.
//      e describes whether the node is an node non-existent in the original graph added by
//      reductions
//          0 means original node,
//          1 means extra node.
// Last m lines describe the edges of the graph in format
// 'u v f' where:
//      u and v are the nodes being connected
//      f is the edge status, 0 means unconstrained, 1 means forced.
// comments starting with c can be only at the beginning of the file.

void Instance::exportADS(std::ostream& output) {
    output << "p ads " << nodeCount() << " " << edgeCount() << " " << ds.size() << "\n";
    for (auto v : ds) output << v << " ";
    output << "\n";

    for (auto v : nodes) {
        output << v << " " << (int)all_nodes[v].domination_status << " " << (int)all_nodes[v].membership_status << " " << all_nodes[v].is_extra << "\n";
    }

    for (auto u : nodes) {
        for (auto [v, status] : all_nodes[u].adj) {
            if (u < v)
                output << u << " " << v << " " << (int)status << "\n";
        }
    }
}


}  // namespace DSHunter