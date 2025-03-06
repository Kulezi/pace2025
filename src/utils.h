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

std::vector<int> intersect(const std::vector<int> a, const std::vector<int> b) {
    DS_ASSERT(is_sorted(a.begin(), a.end()));
    DS_ASSERT(is_sorted(b.begin(), b.end()));

    std::vector<int> res;
    std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(res));
    return res;
}

std::vector<int> unite(const std::vector<int> &a, const std::vector<int> &b) {
    DS_ASSERT(is_sorted(a.begin(), a.end()));
    DS_ASSERT(is_sorted(b.begin(), b.end()));

    std::vector<int> res;
    std::set_union(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(res));
    return res;
}

std::vector<int> unite(const std::vector<int> &&a, const std::vector<int> &&b) {
    DS_ASSERT(is_sorted(a.begin(), a.end()));
    DS_ASSERT(is_sorted(b.begin(), b.end()));

    std::vector<int> res;
    std::set_union(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(res));
    return res;
}

void insert(std::vector<int> &a, int v) {
    DS_ASSERT(is_sorted(a.begin(), a.end()));

    a.insert(std::upper_bound(a.begin(), a.end(), v), v);
    DS_ASSERT(is_sorted(a.begin(), a.end()));
}

void remove(std::vector<int> &a, int v) {
    DS_ASSERT(is_sorted(a.begin(), a.end()));

    auto it = std::lower_bound(a.begin(), a.end(), v);
    if (it != a.end() && *it == v) a.erase(it);

    DS_ASSERT(is_sorted(a.begin(), a.end()));
}

std::vector<int> remove(std::vector<int> a, const std::vector<int> &b) {
    DS_ASSERT(is_sorted(a.begin(), a.end()));
    DS_ASSERT(is_sorted(b.begin(), b.end()));

    for (auto val : b) remove(a, val);

    DS_ASSERT(is_sorted(a.begin(), a.end()));

    return a;
}

std::string dbgv(const std::vector<int> v) {
    std::string s = "[ ";
    for (auto i : v) s += std::to_string(i) + " ";
    s += "]";
    return s;
}
// Returns true if a contains b.
bool contains(const std::vector<int> a, const std::vector<int> b) {
    DS_ASSERT(is_sorted(a.begin(), a.end()));
    DS_ASSERT(is_sorted(b.begin(), b.end()));

    return remove(b, a).empty();
}

std::string pprint(std::vector<int> x) {
    std::string res = "[";
    for (auto i : x) res += std::to_string(i) + " ";
    return res + "]";
}
#endif  // UTILS_H