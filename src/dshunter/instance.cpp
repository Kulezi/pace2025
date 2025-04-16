#include "instance.h"

#include <queue>
#include <set>

#include "utils.h"

namespace DSHunter {

// Constructs graph from input stream assuming DIMACS-like .gr format.
Instance::Instance(std::istream &in) : ds{} {
    std::string line;
    int header_edges, read_edges = 0;
    while (std::getline(in, line)) {
        std::stringstream tokens(line);
        std::string s;
        tokens >> s;
        if (s[0] == 'c') continue;
        if (s[0] == 'p') {
            parse_header(tokens, header_edges);
        } else {
            int a = stoi(s);
            int b = 0;
            tokens >> b;
            ++read_edges;
            all_nodes[a].adj.emplace_back(b, EdgeStatus::UNCONSTRAINED);
            all_nodes[b].adj.emplace_back(a, EdgeStatus::UNCONSTRAINED);
        }
    }

    for (auto v : nodes) sort(all_nodes[v].adj.begin(), all_nodes[v].adj.end());

    if (header_edges != read_edges)
        throw std::logic_error("expected " + std::to_string(header_edges) + " edges, found " +
                               std::to_string(read_edges));
}

void Instance::parse_header(std::stringstream &tokens, int &header_edges) {
    std::string problem;
    int n_nodes = 0;
    tokens >> problem >> n_nodes >> header_edges;

    // if (problem != "ds")
    //     throw std::logic_error("expected problem type to be 'ds', found '" + problem + "'");

    all_nodes.resize(n_nodes + 1,
                     {{}, DominationStatus::UNDOMINATED, InSolutionStatus::MAYBE, false});
    for (int i = 1; i <= n_nodes; ++i) {
        nodes.push_back(i);
    }
}

// Returns the number of nodes in the graph.
size_t Instance::nodeCount() const { return nodes.size(); }

bool Instance::isDominated(int v) const {
    return all_nodes[v].domination_status == DominationStatus::DOMINATED;
}

void Instance::markDominated(int v) {
    DS_TRACE(std::cerr << __func__ << dbg(v) << dbg(c) << std::endl);
    all_nodes[v].domination_status = DominationStatus::DOMINATED;
}

bool Instance::isTaken(int v) const {
    return all_nodes[v].in_solution_status == InSolutionStatus::YES;
}

void Instance::markTaken(int v) {
    all_nodes[v].domination_status = DominationStatus::DOMINATED;
    all_nodes[v].in_solution_status = InSolutionStatus::YES;
}

void Instance::forceEdge(int u, int v) {
    DS_TRACE(std::cerr << __func__ << dbg(u) << dbg(v) << std::endl);
    DS_ASSERT(hasEdge(u, v));
    DS_ASSERT(getEdgeStatus(u, v) != FORCED);
    setEdgeStatus(u, v, EdgeStatus::FORCED);
    markDominated(u);
    markDominated(v);

    // All vertices that see both endpoints of this edge must be dominated by one of them,
    // so we can mark them as dominated.
    auto edgeNeighbourhood = intersect(neighbourhoodExcluding(u), neighbourhoodExcluding(v));
    for (auto w : edgeNeighbourhood) {
        markDominated(w);
    }
}

EdgeStatus Instance::getEdgeStatus(int u, int v) const {
    auto it = lower_bound(all_nodes[u].adj.begin(), all_nodes[u].adj.end(), Endpoint{v, EdgeStatus::ANY});
    DS_ASSERT(it != all_nodes[u].adj.end());
    return it->status;
}

// Returns the degree of given node.
int Instance::deg(int v) const { return (int)all_nodes[v].adj.size(); }

int Instance::forcedDeg(int v) const {
    int res = 0;
    for (auto e : all_nodes[v].adj)
        if (e.status == EdgeStatus::FORCED) res++;
    return res;
}

// Creates and returns the id of the created node.
// Complexity: O(1)
int Instance::addNode() {
    DS_TRACE(std::cerr << __func__ << std::endl);
    int v = all_nodes.size();
    nodes.push_back(all_nodes.size());
    all_nodes.push_back({{}, DominationStatus::UNDOMINATED, InSolutionStatus::MAYBE, true});
    return v;
}

// Removes the node with given id.
// Complexity: O(deg(v) + sum over deg(v) of neighbours)
void Instance::removeNode(int v) {
    DS_TRACE(std::cerr << __func__ << dbg(v) << std::endl);
    if (find(nodes.begin(), nodes.end(), v) == nodes.end()) return;
    for (auto [u, status] : all_nodes[v].adj) {
        // Edges like this can only be removed by calling take().
        DS_ASSERT(status != FORCED || isTaken(v));
        remove(all_nodes[u].adj, Endpoint{v, status});
        DS_ASSERT(is_sorted(all_nodes[u].adj.begin(), all_nodes[u].adj.end()));
    }
    all_nodes[v].adj.clear();

    remove(nodes, v);
    DS_ASSERT(std::is_sorted(all_nodes[v].adj.begin(), all_nodes[v].adj.end()));
}

// Removes nodes in the given list from the graph.
// Complexity: O(sum of deg(v) over l ∪ N(l))
void Instance::removeNodes(const std::vector<int> &l) {
    for (auto &v : l) removeNode(v);
}

// Adds an unconstrained edge between nodes with id's u and v.
// Complexity: O(deg(v)), due to maintaining adjacency list to be sorted.
void Instance::addEdge(int u, int v) {
    DS_TRACE(std::cerr << __func__ << dbg(u) << dbg(v) << std::endl);
    insert(all_nodes[u].adj, Endpoint{v, EdgeStatus::UNCONSTRAINED});
    insert(all_nodes[v].adj, Endpoint{u, EdgeStatus::UNCONSTRAINED});
}

// Removes edge (v, w) from the graph.
// Complexity: O(deg(v) + deg(w))
void Instance::removeEdge(int v, int w) {
    DS_TRACE(std::cerr << __func__ << dbg(v) << dbg(w) << std::endl);
    remove(all_nodes[v].adj, {w, EdgeStatus::ANY});
    remove(all_nodes[w].adj, {v, EdgeStatus::ANY});
}

// Same meaning as N[v] notation.
// Complexity: O(deg(v)).
const std::vector<int> Instance::neighbourhoodIncluding(int v) const {
    std::vector<int> res(all_nodes[v].adj.size());
    for (size_t i = 0; i < all_nodes[v].adj.size(); ++i) {
        res[i] = all_nodes[v].adj[i].to;
    }
    insert(res, v);
    return res;
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

// Same meaning as N(v) notation.
// Complexity: O(1)
std::vector<int> Instance::neighbourhoodExcluding(int v) const {
    std::vector<int> res(all_nodes[v].adj.size());
    for (size_t i = 0; i < all_nodes[v].adj.size(); ++i) {
        res[i] = all_nodes[v].adj[i].to;
    }
    return res;
}

// Returns true if and only if undirected edge (u, v) is present in the graph.
// Complexity: O(deg(u)) !
bool Instance::hasEdge(int u, int v) const {
    return std::find(all_nodes[u].adj.begin(), all_nodes[u].adj.end(), Endpoint{v, EdgeStatus::ANY}) !=
           all_nodes[u].adj.end();
}

// Inserts given node to the dominating set, changing the status of
// it's neighours to DOMINATED if they are not, the node is removed from the graph afterwards.
// Complexity: O(deg(v)) or O(sum of degrees of neighbours) in case of extra vertices.
void Instance::take(int v) {
    DS_TRACE(std::cerr << __func__ << dbg(v) << std::endl);
    DS_ASSERT(!isTaken(v));
    if (all_nodes[v].is_extra) {
        for (auto u : neighbourhoodExcluding(v)) {
            DS_ASSERT(!isTaken(u));
            take(u);
        }

        return;
    }
    all_nodes[v].in_solution_status = InSolutionStatus::YES;

    ds.push_back(v);
    for (auto u : neighbourhoodExcluding(v)) {
        DS_ASSERT(!isTaken(u));
        markDominated(u);
    }

    removeNode(v);
}

// Splits the list of graph nodes into individual connected components.
// Note in case of a connected graph it returns an empty list.
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

                for (auto [u, _] : all_nodes[w].adj) {
                    if (component[u] < 0) {
                        component[u] = components;
                        q.push(u);
                    }
                }
            }

            ++components;
        }
    }

    if (components <= 1) return {};
    std::vector<std::vector<int>> result(components, std::vector<int>());
    for (auto v : nodes) {
        result[component[v]].push_back(v);
    }
    return result;
}

void Instance::setEdgeStatus(int u, int v, EdgeStatus status) {
    auto it_u = lower_bound(all_nodes[u].adj.begin(), all_nodes[u].adj.end(), Endpoint{v, EdgeStatus::ANY});
    DS_ASSERT(it_u != adj[u].end());
    auto it_v = lower_bound(all_nodes[v].adj.begin(), all_nodes[v].adj.end(), Endpoint{u, EdgeStatus::ANY});
    DS_ASSERT(it_v != adj[v].end());

    it_u->status = it_v->status = status;
}

const Node &Instance::operator[](int v) const { return all_nodes[v]; }

}  // namespace DSHunter