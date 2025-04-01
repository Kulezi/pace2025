#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "ds.h"
#include "instance.h"
#include "rrules.h"

// This test checks whether a brute-force solution gives the same result as the model solution
// on all graphs with at most 7 vertices.
int main() {
    DSHunter::Exact sol(DSHunter::RRules::defaults_preprocess, DSHunter::RRules::defaults_branching);
    DSHunter::Exact brute({}, {});

    std::ofstream trash("/dev/null");
    for (int n = 1; n <= 7; n++) {
        int max_edges = n * (n - 1) / 2;
        for (int mask = 0; mask < (1 << max_edges); mask++) {
            // if (mask / (1 << max_edges) > (mask-1) / (1 << max_edges)) std::cerr << mask / (1 << max_edges) << "% finished" << std::endl;
            std::cerr << "\rn=" << n << ", graph " << mask+1 << " out of " <<  (1<<max_edges) <<  std::flush;
            auto print_graph = [&](std::ostream &out) {
                out << "p ds " << n << " " << __builtin_popcount(mask) << "\n";
                for (int i = 1, e = 0; i <= n; i++) {
                    for (int j = i + 1; j <= n; j++, e++) {
                        if (mask >> e & 1) out << i << " " << j << "\n";
                    }
                }
            };

            std::stringstream g_str;
            print_graph(g_str);

            DSHunter::Instance g(g_str);

            try {
                DS_TRACE(print_graph(std::cerr));
                auto sol_brute = brute.solveBruteforce(g, trash);
                auto sol_main = sol.solve(g, trash);

                if (sol_brute.size() != sol_main.size()) {
                    std::cerr << "found ds of size " << sol_main.size() << ", expected "
                              << sol_brute.size() << "\n";

                    return 1;
                }
            } catch (std::logic_error &e) {
                print_graph(std::cerr);

                std::cerr << e.what() << "\n";
                return 1;
            }
        }

        std::cerr << "\r[OK] for all " << (1 << max_edges) << " graphs with n = " << n << "\n";
    }
}
