#ifndef INSTANCE_H
#define INSTANCE_H
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
namespace DSHunter {

enum class DominationStatus {
    UNDOMINATED,
    DOMINATED
};

enum class MembershipStatus {
    UNDECIDED,
    DISREGARDED,
    TAKEN
};

enum class EdgeStatus {
    UNCONSTRAINED,
    FORCED,
    ANY
};

struct Endpoint {
    int to;
    EdgeStatus status;

    // Multiple edges are disallowed, so we only need to compare the ends of the edge.
    bool operator<(const Endpoint &rhs) const { return to < rhs.to; }
    bool operator==(const Endpoint &rhs) const { return to == rhs.to; };
};

struct Node {
    Node();
    Node(int v, bool is_extra = true);

    // List of adjacent nodes sorted by increasing node id.
    // Order is maintained to make set union/intersection possible in O(|A| + |B|).
    std::vector<Endpoint> adj;
    std::vector<int> n_open;
    std::vector<int> n_closed;
    std::vector<int> dominators;
    std::vector<int> dominatees;

    DominationStatus domination_status;
    MembershipStatus membership_status;

    // Extra vertices cannot be taken into the dominating set, we assume they mean if we take them
    // we should take all their neighbours instead.
    bool is_extra;
};

// Undirected graph representing an instance of dominating set problem.
// Nodes are marked with a domination status.
// Node labels are assigned incrementally starting with 1.
struct Instance {
    // List of active node ids sorted increasingly.
    std::vector<int> nodes;

    std::vector<Node> all_nodes;

    // Nodes already removed from the graph considered as the dominating set candidates.
    std::vector<int> ds;

    std::vector<std::vector<int>> sets_to_hit;
    // Constructs an empty graph.
    Instance();

    // Constructs graph from input stream assuming DIMACS-like .gr format.
    Instance(std::istream &in);

    // Returns the number of nodes in the graph.
    size_t nodeCount() const;
    size_t disregardedNodeCount() const;

    // Returns the number of edges in the graph.
    // Complexity: O(n)
    int edgeCount() const;

    bool isDominated(int v) const;
    void markDominated(int v);

    bool isTaken(int v) const;
    void markTaken(int v);

    bool isDisregarded(int v) const;
    void markDisregarded(int v);

    // Returns the degree of given node.
    int deg(int v) const;

    // Returns the count of forced edges adjacent to given node.
    int forcedDeg(int v) const;

    // Creates and returns the id of the created node.
    // Complexity: O(1)
    int addNode();

    bool hasNode(int v) const;

    // Removes the node with given id.
    // Complexity: O(deg(v) + sum over deg(v) of neighbours)
    void removeNode(int v);

    // Removes nodes in the given list from the graph.
    // Complexity: O(sum of deg(v) over l ∪ N(l))
    void removeNodes(const std::vector<int> &l);

    // Inserts given node to the dominating set, changing the status of
    // it's neighours to DOMINATED if they are not, the node is removed from the graph afterwards.
    // Complexity: O(deg(v)) or O(sum of degrees of neighbours) in case of extra vertices.
    void take(int v);

    // Adds an unconstrained edge between nodes with id's u and v.
    // Complexity: O(deg(v)), due to maintaining adjacency list to be sorted.
    void addEdge(int u, int v, EdgeStatus status = EdgeStatus::UNCONSTRAINED);

    // Removes edge (v, w) from the graph.
    // Complexity: O(deg(v) + deg(w))
    void removeEdge(int v, int w);

    void forceEdge(int u, int v);
    EdgeStatus getEdgeStatus(int u, int v) const;

    int forcedEdgeCount() const;

    // Returns true if and only if undirected edge (u, v) is present in the graph.
    // Complexity: O(deg(u)) !
    bool hasEdge(int u, int v) const;

    // Splits the list of graph nodes into individual connected components.
    // Note in case of a connected graph it returns an empty list.
    // Complexity: O(n + m)
    std::vector<std::vector<int>> split() const;

    const Node &operator[](int v) const;
    void exportADS(std::ostream& output);

   private:
    void setEdgeStatus(int u, int v, EdgeStatus status);

    void addDirectedEdge(int u, int v, EdgeStatus status);
    void removeDirectedEdge(int u, int v);

    void initAddEdge(int u, int v, EdgeStatus status = EdgeStatus::UNCONSTRAINED);
    void initAddDirectedEdge(int u, int v, EdgeStatus status = EdgeStatus::UNCONSTRAINED);
    void sortAdjacencyLists();

    void parseDS(std::istream &in, int n_nodes, int header_edges);
    void parseADS(std::istream &in, int n_nodes, int header_edges, int d);
};
}  // namespace DSHunter
#endif  // INSTANCE_H
