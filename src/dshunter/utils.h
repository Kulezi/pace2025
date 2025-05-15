
#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

#ifdef DS_TESTING_MODE
#define DS_ASSERT(cond) assert(cond)
#else
#define DS_ASSERT(cond)
#endif

#ifdef DS_TRACING_MODE
#define DS_TRACE(x) x
#else
#define DS_TRACE(x)
#endif

#define dbg(x) " " << #x << " = " << x << " "
#define dbgv(x) " " << #x << " = " << DSHunter::stringify(x) << " "

namespace DSHunter {

// Computes A ∩ B.
// Complexity: O(|A| + |B|)
template <typename T>
std::vector<T> intersect(const std::vector<T> a, const std::vector<T> b) {
    DS_ASSERT(is_sorted(a.begin(), a.end()));
    DS_ASSERT(is_sorted(b.begin(), b.end()));

    std::vector<T> res;
    std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(res));
    return res;
}

// Computes A ∪ B.
// Complexity: O(|A| + |B|)
template <typename T>
std::vector<T> unite(const std::vector<T> &a, const std::vector<T> &b) {
    DS_ASSERT(is_sorted(a.begin(), a.end()));
    DS_ASSERT(is_sorted(b.begin(), b.end()));

    std::vector<T> res;
    std::set_union(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(res));
    return res;
}

// Computes A ∪ B.
// Complexity: O(|A| + |B|)
template <typename T>
std::vector<T> unite(const std::vector<T> &&a, const std::vector<T> &&b) {
    DS_ASSERT(is_sorted(a.begin(), a.end()));
    DS_ASSERT(is_sorted(b.begin(), b.end()));

    std::vector<T> res;
    std::set_union(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(res));
    return res;
}

// Inserts v into a preserving ascending order.
// Complexity: O(|A|)
template <typename T>
void insert(std::vector<T> &a, T v) {
    DS_ASSERT(is_sorted(a.begin(), a.end()));

    a.insert(std::upper_bound(a.begin(), a.end(), v), v);
    DS_ASSERT(is_sorted(a.begin(), a.end()));
}

// Computes A \ {v}.
// Complexity: O(|A|)
template <typename T>
void remove(std::vector<T> &a, T v) {
    DS_ASSERT(is_sorted(a.begin(), a.end()));

    auto it = std::lower_bound(a.begin(), a.end(), v);
    if (it != a.end() && *it == v)
        a.erase(it);

    DS_ASSERT(is_sorted(a.begin(), a.end()));
}

// Computes A \ B.
// Complexity: O(|A| + |B|)
template <typename T>
std::vector<T> remove(std::vector<T> a, const std::vector<T> &b) {
    DS_ASSERT(is_sorted(a.begin(), a.end()));
    DS_ASSERT(is_sorted(b.begin(), b.end()));

    std::vector<T> res;

    // Pointers to smallest unhandled elements.
    size_t i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        if (a[i] < b[j]) {
            res.push_back(a[i]);
            ++i;
        } else if (a[i] == b[j]) {
            ++i;
            ++j;
        } else if (b[j] < a[i]) {
            ++j;
        }
    }

    while (i < a.size()) res.push_back(a[i++]);

    DS_ASSERT(is_sorted(res.begin(), res.end()));

    return res;
}

// Returns a human-readable string representing a given vector.
std::string stringify(const std::vector<int> &v);

// Returns true if a contains b.
// Complexity: O(|A| + |B|).
template <typename T>
bool contains(const std::vector<T> a, const std::vector<T> b) {
    DS_ASSERT(is_sorted(a.begin(), a.end()));
    DS_ASSERT(is_sorted(b.begin(), b.end()));

    return remove(b, a).empty();
}
}  // namespace DSHunter

#endif  // UTILS_H