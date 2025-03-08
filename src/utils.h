#ifndef UTILS_H
#define UTILS_H
#include <algorithm>
#include <string>
#include <vector>

#ifdef DEBUG_MODE
#define DS_ASSERT(cond) assert(cond)
#else
#define DS_ASSERT(cond)
#endif

#define dbg(x) #x << " = " << x << " "

// Computes A ∩ B.
// Complexity: O(|A| + |B|)
std::vector<int> intersect(const std::vector<int> a, const std::vector<int> b) {
    DS_ASSERT(is_sorted(a.begin(), a.end()));
    DS_ASSERT(is_sorted(b.begin(), b.end()));

    std::vector<int> res;
    std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(res));
    return res;
}

// Computes A ∪ B.
// Complexity: O(|A| + |B|)
std::vector<int> unite(const std::vector<int> &a, const std::vector<int> &b) {
    DS_ASSERT(is_sorted(a.begin(), a.end()));
    DS_ASSERT(is_sorted(b.begin(), b.end()));

    std::vector<int> res;
    std::set_union(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(res));
    return res;
}

// Computes A ∪ B.
// Complexity: O(|A| + |B|)
std::vector<int> unite(const std::vector<int> &&a, const std::vector<int> &&b) {
    DS_ASSERT(is_sorted(a.begin(), a.end()));
    DS_ASSERT(is_sorted(b.begin(), b.end()));

    std::vector<int> res;
    std::set_union(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(res));
    return res;
}

// Inserts v into a preserving ascending order.
// Complexity: O(|A|)
void insert(std::vector<int> &a, int v) {
    DS_ASSERT(is_sorted(a.begin(), a.end()));

    a.insert(std::upper_bound(a.begin(), a.end(), v), v);
    DS_ASSERT(is_sorted(a.begin(), a.end()));
}

// Computes A \ {v}.
// Complexity: O(|A|)
void remove(std::vector<int> &a, int v) {
    DS_ASSERT(is_sorted(a.begin(), a.end()));

    auto it = std::lower_bound(a.begin(), a.end(), v);
    if (it != a.end() && *it == v) a.erase(it);

    DS_ASSERT(is_sorted(a.begin(), a.end()));
}

// Computes A \ B.
// Complexity: O(|A| + |B|)
std::vector<int> remove(std::vector<int> a, const std::vector<int> &b) {
    DS_ASSERT(is_sorted(a.begin(), a.end()));
    DS_ASSERT(is_sorted(b.begin(), b.end()));

    std::vector<int> res;
    
    // Pointers to smallest unhandled elements.
    size_t i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        if (a[i] < b[j]) {
            res.push_back(a[i]);
            ++i;
        } else if (a[i] == b[j]) {
            ++i;
            ++j;
        } else if (a[i] > b[j]) {
            ++j;
        }
    }

    while (i < a.size()) res.push_back(a[i++]);

    DS_ASSERT(is_sorted(res.begin(), res.end()));

    return res;
}

// Returns a human-readable string representing given vector.
std::string dbgv(const std::vector<int> v) {
    std::string s = "[ ";
    for (auto i : v) s += std::to_string(i) + " ";
    s += "]";
    return s;
}

// Returns true if a contains b.
// Complexity: O(|A| + |B|).
bool contains(const std::vector<int> a, const std::vector<int> b) {
    DS_ASSERT(is_sorted(a.begin(), a.end()));
    DS_ASSERT(is_sorted(b.begin(), b.end()));

    return remove(b, a).empty();
}

#endif  // UTILS_H