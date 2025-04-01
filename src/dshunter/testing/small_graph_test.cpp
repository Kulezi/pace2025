#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "../dshunter.h"

// This test checks whether a brute-force solution gives the same result as the model solution
// on all graphs with at most 7 vertices.
int main() {
    DSHunter::Solver brute_reductionless(DSHunter::SolverConfig(DSHunter::default_reduction_rules,
                                                                DSHunter::SolverType::Bruteforce,
                                                                DSHunter::PresolverType::None));
    DSHunter::Solver brute_reduce(DSHunter::SolverConfig(DSHunter::default_reduction_rules,
                                                         DSHunter::SolverType::Bruteforce,
                                                         DSHunter::PresolverType::Full));
    DSHunter::Solver default_solver;

    for (int n = 1; n <= 7; n++) {
        int max_edges = n * (n - 1) / 2;
        for (int mask = 0; mask < (1 << max_edges); mask++) {
            std::cerr << "\rn=" << n << ", graph " << mask + 1 << " out of " << (1 << max_edges)
                      << std::flush;
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
                auto sol_brute_reductionless = brute_reductionless.solve(g);
                auto sol_brute_reduce = brute_reduce.solve(g);
                auto sol = default_solver.solve(g);

                if (sol_brute_reduce.size() != sol_brute_reductionless.size()) {
                    std::cerr << "brute_reduce found ds of size " << sol_brute_reduce.size() << ", expected "
                              << sol_brute_reductionless.size() << "\n";

                    return 1;
                }                

                if (sol_brute_reduce.size() != sol_brute_reductionless.size()) {
                    std::cerr << "default_solver found ds of size " << sol_brute_reduce.size() << ", expected "
                              << sol_brute_reductionless.size() << "\n";
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
