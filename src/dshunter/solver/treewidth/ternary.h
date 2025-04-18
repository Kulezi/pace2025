#ifndef TERNARY_H
#define TERNARY_H
#include <string>

namespace DSHunter {

constexpr int MAX_EXPONENT = 18;
constexpr size_t pow3[MAX_EXPONENT + 1] = {
    1,
    3,
    9,
    27,
    81,
    243,
    729,
    2187,
    6561,
    19683,
    59049,
    177147,
    531441,
    1594323,
    4782969,
    14348907,
    43046721,
    129140163,
    387420489,
};

// e.g. WHITE = 0, GRAY = 0_dash, BLACK = 1 in platypus book.
enum class Color {
    WHITE,
    GRAY,
    BLACK
};

using TernaryFun = size_t;

// Remove x'th argument of f from the domain.
TernaryFun cut(TernaryFun f, size_t x);

// Insert at position x.
TernaryFun insert(TernaryFun f, size_t x, Color c);
TernaryFun set(TernaryFun f, size_t x, Color c);

Color at(TernaryFun f, size_t x);
char val(Color c);

TernaryFun toInt(std::string s);
std::string toString(TernaryFun f);
}  // namespace DSHunter
#endif