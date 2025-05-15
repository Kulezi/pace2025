#ifndef INSTANCE_H
#define INSTANCE_H
#include <sstream>
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
    explicit Node(int v, bool is_extra = true);

    // List of adjacent nodes sorted by increasing node id.
    // Order is maintained to make set union/intersection possible in O(|A| + |B|).
    std::vector<Endpoint> adj;
    std::vector<int> n_open;
    std::vector<int> n_closed;
    std::vector<int> dominators;
    std::vector<int> dominatees;

    DominationStatus domination_status;
    MembershipStatus membership_status;

    // Extra vertices cannot be taken into the dominating set.
    // Taking it has an effect of taking of its neighbors instead.
    bool is_extra;
};

// Undirected graph representing an instance of the dominating set problem.
// Nodes are marked with a domination status.
// Node labels are assigned incrementally starting with 1.
struct Instance {
    virtual ~Instance() = default;

    // List of active node ids sorted increasingly.
    std::vector<int> nodes;

    std::vector<Node> all_nodes;

    std::vector<int> ds;

    std::vector<std::vector<int>> sets_to_hit;

    // Constructs an empty graph.
    Instance();

    // Constructs graph from the input stream assuming DIMACS-like .gr format.
    explicit Instance(std::istream &in);

    // Returns the number of nodes in the graph.
    [[nodiscard]] int nodeCount() const;
    [[nodiscard]] int disregardedNodeCount() const;

    // Returns the number of edges in the graph.
    // Complexity: O(n)
    [[nodiscard]] int edgeCount() const;

    [[nodiscard]] bool isDominated(int v) const;
    void markDominated(int v);

    [[nodiscard]] bool isTaken(int v) const;
    void markTaken(int v);

    [[nodiscard]] bool isDisregarded(int v) const;
    void markDisregarded(int v);
    void ignore(int v);

    // Returns the degree of the given node.
    [[nodiscard]] int deg(int v) const;

    // Returns the count of forced edges adjacent to the given node.
    [[nodiscard]] int forcedDeg(int v) const;

    // Creates and returns the id of the created node.
    // Complexity: O(1)
    int addNode();

    [[nodiscard]] bool hasNode(int v) const;

    // Removes the node with the given id from the graph.
    // Complexity: O(deg(v) + sum over deg(v) of neighbors)
    virtual void removeNode(int v);

    // Removes nodes in the given list from the graph.
    // Complexity: O(sum of deg(v) over l ∪ N(l))
    void removeNodes(const std::vector<int> &l);

    // Inserts v to the dominating set, dominating its neighbors and removing v from the graph.
    // Complexity: O(deg(v)) or O(sum of degrees of neighbors) in case of extra vertices.
    void take(int v);

    // Adds an unconstrained edge between nodes with id's u and v.
    // Complexity: O(deg(v))
    void addEdge(int u, int v, EdgeStatus status = EdgeStatus::UNCONSTRAINED);

    // Removes edge (v, w) from the graph.
    // Complexity: O(deg(v) + deg(w))
    void removeEdge(int v, int w);

    void forceEdge(int u, int v);
    [[nodiscard]] EdgeStatus getEdgeStatus(int u, int v) const;

    [[nodiscard]] int forcedEdgeCount() const;

    // Returns true if and only if an undirected edge (u, v) is present in the graph.
    // Complexity: O(deg(u)) !
    [[nodiscard]] bool hasEdge(int u, int v) const;

    // Splits the list of graph nodes into individual connected components.
    // Complexity: O(n + m)
    [[nodiscard]] std::vector<std::vector<int>> split() const;

    const Node &operator[](int v) const;
    /*
    .ads format description:
     First line is 'p ads n m d' where:
         - n is the number of remaining nodes
         - m is the number of remaining edges
         - d is the number of nodes already known to be in the optimal dominating set
         (those nodes are guaranteed to not be present in the graph)
     Second line is v_1, v_2, ..., v_d, being the list of nodes already known to be in the optimal
     dominating set following it are n lines describing the nodes of the remaining graph in format 'v
     s_d s_m e', where:
         - v is the node number in the original graph, note nodes may not be numbered from 1 to n.
         - s_d is the nodes domination status, 0 means undominated, 1 means dominated
         - s_m is the nodes solution membership status, 0 means maybe, 1 means no, 2 means yes
         - e describes whether the node is a node non-existent in the original graph added by
         reductions
             0 means original node,
             1 means extra node.
    The last m lines describe the edges of the graph in format 'u v f' where:
         - u and v are the nodes being connected
         - f is the edge status, 0 means unconstrained, 1 means forced.
    Comments starting with c can be only at the beginning of the file.
    */
    void exportADS(std::ostream &output);

   private:
    void setEdgeStatus(int u, int v, EdgeStatus status);

    void addDirectedEdge(int u, int v);
    void removeDirectedEdge(int u, int v);

    void initAddEdge(int u, int v, EdgeStatus status = EdgeStatus::UNCONSTRAINED);
    void initAddDirectedEdge(int u, int v, EdgeStatus status = EdgeStatus::UNCONSTRAINED);
    void sortAdjacencyLists();

    void parseDS(std::istream &in, int n_nodes, int header_edges);
    void parseADS(std::istream &in, int n_nodes, int header_edges, int d);
};
}  // namespace DSHunter
#endif  // INSTANCE_H
