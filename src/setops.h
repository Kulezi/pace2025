#ifndef _SETOPS_H
#define _SETOPS_H
#include <list>
#include <algorithm>
#include <string>
std::list<int> intersect(std::list<int> &a, std::list<int> &b) {
    std::list<int> res;
    std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(res));
    return res;
}

std::list<int> unite(std::list<int> &a, std::list<int> &b) {
    std::list<int> res;
    std::set_union(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(res));
    return res;
}

std::list<int> unite(std::list<int> &&a, std::list<int> &&b) {
    std::list<int> res;
    std::set_union(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(res));
    return res;
}

std::list<int> remove(std::list<int> a, std::list<int> &b) {
    for (auto val : b) a.remove(val);
    return a;
}

std::string pprint(std::list<int> x) {
    std::string res = "[";
    for (auto i : x) res += std::to_string(i) + " ";
    return res + "]";
}
#endif