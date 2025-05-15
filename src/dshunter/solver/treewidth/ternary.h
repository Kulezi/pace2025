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

// e.g., WHITE = 0, GRAY = 0_dash, BLACK = 1 like in the platypus book.
enum class Color {
    WHITE,
    GRAY,
    BLACK
};

using TernaryFun = size_t;

// Removes xth argument of f from the domain.
TernaryFun cut(TernaryFun f, size_t x);

// Insert c at position x.
TernaryFun insert(TernaryFun f, size_t x, Color c);

// Set xth trit to c
TernaryFun set(TernaryFun f, size_t x, Color c);

// Set xth trit of f to c assuming it was zero before, we omit unnecessary division then.
TernaryFun setUnset(TernaryFun f, size_t x, Color c);

// Return the value of f's xth trit.
Color at(TernaryFun f, size_t x);

char val(Color c);
TernaryFun toInt(std::string s);
std::string toString(TernaryFun f);
}  // namespace DSHunter
#endif