#include "utils.h"

namespace DSHunter {
// Returns a human-readable string representing given vector.
std::string to_string(const std::vector<int> &v) {
    std::string s = "[ ";
    for (auto i : v) s += std::to_string(i) + " ";
    s += "]";
    return s;
}
}