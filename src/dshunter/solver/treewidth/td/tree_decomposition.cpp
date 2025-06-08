#include "tree_decomposition.h"

#include "../../../utils.h"
namespace DSHunter {
int TreeDecomposition::size() const { return bag.size(); }

void TreeDecomposition::print() const {
    std::cerr << "width: " << width << std::endl;
    std::cerr << "size: " << size() << std::endl;
    for (int i = 0; i < size(); i++) {
        std::cerr << i << ":" << dbgv(bag[i]) << dbgv(adj[i]) << std::endl;
    }
}

void TreeDecomposition::addEdge(int a, int b) {
    DS_ASSERT(find(adj[a].begin(), adj[a].end(), b) == adj[a].end());
    DS_ASSERT(find(adj[b].begin(), adj[b].end(), a) == adj[b].end());
    adj[a].push_back(b);
    adj[b].push_back(a);
}

int TreeDecomposition::biggestBag() const {
    int max_bag = 0;
    for (int i = 0; i < size(); i++) {
        if (bag[i].size() > bag[max_bag].size()) {
            max_bag = i;
        }
    }
    return max_bag;
}
void TreeDecomposition::removeNode(int v) {
    for (int i = 0; i < size(); i++) {
        auto pos = std::ranges::find(bag[i], v);
        if (pos != bag[i].end())
            bag[i].erase(pos);
    }
}

}  // namespace DSHunter
