#include <iostream>

#include "ds.h"
#include "instance.h"
#include "rrules.h"
int main() {
    DSHunter::Exact ds(DSHunter::RRules::defaults_preprocess, DSHunter::RRules::defaults_preprocess);

    DSHunter::Instance g(std::cin);
    auto ans = ds.solve(g, std::cout);
}
