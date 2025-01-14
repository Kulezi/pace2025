#include <iostream>
#include <fstream>
#include "ds.h"
#include "instance.h"
#include "rrules.h"
#include "tw.h"
int main() {
    std::ifstream f("/home/dvdpawcio/pace2025/in/tiny/testset/test.gr");
    Instance g(f);
    g.print();
    TreeDecomposition t(g);
}
