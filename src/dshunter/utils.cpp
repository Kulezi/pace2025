#include "utils.h"

namespace DSHunter {
std::string stringify(const std::vector<int> &v) {
    std::string s = "[ ";
    for (auto i : v) s += std::to_string(i) + " ";
    s += "]";
    return s;
}
}