#ifndef _SETOPS_H
#define _SETOPS_H
#include <algorithm>
#include <string>
#include <vector>

template<typename T>
std::vector<T> intersect(std::vector<T> a, std::vector<T> b) {
    std::vector<T> res;
    std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(res));
    return res;
}
template<typename T>
std::vector<T> unite(std::vector<T> &a, std::vector<T> &b) {
    std::vector<T> res;
    std::set_union(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(res));
    return res;
}
template<typename T>
std::vector<T> unite(std::vector<T> &&a, std::vector<T> &&b) {
    std::vector<T> res;
    std::set_union(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(res));
    return res;
}

template<typename T>
void remove(std::vector<T> &a, T v) {
    auto it = std::lower_bound(a.begin(), a.end(), v);
    if (it != a.end() && *it == v) a.erase(it);
    assert(is_sorted(a.begin(),a.end()));
}

template<typename T>
std::vector<T> remove(std::vector<T> a, std::vector<T> &b) {
    for (auto val : b) remove(a, val);
    assert(is_sorted(a.begin(),a.end()));

    return a;
}

// Returns true if a contains b.
template<typename T>
bool contains(std::vector<T> a, std::vector<T> b) {
    return remove(b, a).empty();
}

template<typename T>
void insert(std::vector<T> &a, int v) { a.insert(std::upper_bound(a.begin(), a.end(), v), v); 
    assert(is_sorted(a.begin(),a.end()));
}


template<typename T>
std::string dbgv(std::vector<T> v) {
    std::string s = "[ ";
    for (auto i : v) s += std::to_string(i) + " ";
    s += "]";
    return s;
}

template<typename T>
std::string pprint(std::vector<T> x) {
    std::string res = "[";
    for (auto i : x) res += std::to_string(i) + " ";
    return res + "]";
}
#endif