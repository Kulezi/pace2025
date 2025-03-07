#include <chrono>
#include <iostream>

#include "ds.h"
#include "nice_tree_decomposition.h"
#include "instance.h"

#include "rrules.h"

#define dbg(x) #x << " = " << x << " "
int main() {
    Instance g(std::cin);
   
    NiceTreeDecomposition td(g);
    td.print();
}
