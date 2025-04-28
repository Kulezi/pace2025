#include "ternary.h"

#include "../../utils.h"

namespace DSHunter {
// Remove x'th argument of f from the domain.
TernaryFun cut(TernaryFun f, size_t x) {
    DS_ASSERT(x <= MAX_EXPONENT);
    // Value of the first x-1 trits.
    TernaryFun pref = f % pow3[x];

    // Value of trits x+1, x+2, ...
    TernaryFun suf = (f / pow3[x + 1]) * pow3[x];

    return pref + suf;
}

// Insert at position x.
TernaryFun insert(TernaryFun f, size_t x, Color c) {
    DS_ASSERT(x <= MAX_EXPONENT);
    // Value of the first x-1 trits.
    TernaryFun pref = f % pow3[x];
    // Every trit outside of prefix gets shifted right.
    TernaryFun suf = (f - pref) * 3;

    // Lastly we add the value of inserted trit.
    return pref + suf + (size_t)c * pow3[x];
}

TernaryFun set(TernaryFun f, size_t x, Color c) {
    DS_ASSERT(x <= MAX_EXPONENT);
    return f + ((int)c - (int)at(f, x)) * pow3[x];
}

TernaryFun setUnset(TernaryFun f, size_t x, Color c) {
    DS_ASSERT(x <= MAX_EXPONENT);
    return f + (int)c * pow3[x];
}

Color at(TernaryFun f, size_t x) {
    DS_ASSERT(x <= MAX_EXPONENT);
    return (Color)(f / pow3[x] % 3);
}

char val(Color c) {
    if ((int)c == 0)
        return '0';
    if ((int)c == 1)
        return '^';
    return '1';
}

TernaryFun toInt(std::string s) {
    TernaryFun res = 0;
    for (size_t i = 0; i < s.size(); i++) {
        res += pow3[i] * static_cast<size_t>(s[i] - '0');
    }
    return res;
}

std::string toString(TernaryFun f) {
    if (f == 0)
        return "0";
    std::string res;
    while (f > 0) {
        res += (char)((f % 3) + '0');
        f /= 3;
    }

    return res;
}
}  // namespace DSHunter