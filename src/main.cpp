#include <getopt.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>

#include "dshunter/dshunter.h"
#include "dshunter/solver/treewidth/treewidth_solver.h"
namespace {
void print_help() {
    std::cout
        << "Usage:\n"
        << "  dshunter [--input_file <graph.gr/graph.ads>]\n"
        << "           [--output_file <file.ds>]\n"
        << "           [--solver <bruteforce/branching/treewidth_dp/mip/vc/gurobi>]\n"
        << "           [--decomposer] <decomposer executable>\n"
        << "           [--mode] <presolve/ds_size/treewidth>\n"
        << "           [--presolve <full/cheap/none>]\n"
        << "           [--short]\n"
        << "           [--help]\n\n"

        << "Options:\n"
        << "  --input_file    Read instance from specified file (default: stdin)\n"
        << "  --output_file   Write solution to specified file (default: stdout)\n"
        << "  --solver        Choose solving method: bruteforce, branching, treewidth_dp, "
           "mip, vc, gurobi\n"
        << "  --decomposer    Use external executable to get tree decompositions\n"
        << "  --mode          Picks one of the non-default output modes for the solver\n"
        << "  --presolve      Choose presolver: full, cheap, none\n"
        << "  --help          Show this help message and exit\n\n"

        << "By default dshunter reads the instance in .gr format from stdin.\n"
        << "--input_file flag overrides this behaviour making it read from a file.\n\n"

        << "To force dshunter to use a certain method of solving the instance\n"
        << "--solver flag can be used with its respective value.\n\n"

        << "By default dshunter will decide by itself whether to presolve the instance or not.\n"
        << "--presolve flag can be used to force certain presolver behaviour.\n\n"

        << "By default dshunter will print full solution in PACE2025 Dominating Set solution "
           "format.\n"
        << "This can be overriden by passing the --mode flag with one of the following options:\n"
        << "    --mode ds_size makes dshunter output only the solution set size.\n"
        << "    --mode presolve makes dshunter output only the instance after presolving.\n"
        << "    --mode treewidth makes dshunter output only the instance treewidth after "
           "presolving.\n"
        << "    --mode histogram makes dshunter output only a histogram of sizes of bags in a nice "
           "decomposition after presolving\n\n";
    exit(EXIT_SUCCESS);
}

enum SolverMode {
    SOLUTION,
    PRESOLUTION,
    SOLUTION_SIZE,
    TREEWIDTH,
    HISTOGRAM,
};

void parse_arguments(int argc, char* argv[], std::string& input_file, std::string& output_file, DSHunter::SolverConfig& config, SolverMode& mode) {
    struct option long_options[] = { { "input_file", required_argument, nullptr, 'i' },
                                     { "output_file", required_argument, nullptr, 'o' },
                                     { "solver", required_argument, nullptr, 's' },
                                     { "decomposer", required_argument, nullptr, 'd' },
                                     { "mode", required_argument, nullptr, 'm' },
                                     { "presolve", required_argument, nullptr, 'p' },
                                     { "help", no_argument, nullptr, 'h' },
                                     { nullptr, 0, nullptr, 0 } };

    int opt;
    while ((opt = getopt_long(argc, argv, "i:o:m:p:sh", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'i':
                input_file = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            case 's':
                if (std::string(optarg) == "bruteforce")
                    config.solver_type = DSHunter::SolverType::Bruteforce;
                else if (std::string(optarg) == "branching")
                    config.solver_type = DSHunter::SolverType::Branching;
                else if (std::string(optarg) == "treewidth_dp")
                    config.solver_type = DSHunter::SolverType::TreewidthDP;
                else if (std::string(optarg) == "vc")
                    config.solver_type = DSHunter::SolverType::ReduceToVertexCover;
                else if (std::string(optarg) == "gurobi")
                    config.solver_type = DSHunter::SolverType::Gurobi;
                else
                    throw std::logic_error(std::string(optarg) + " is not a valid --solver value");
                break;
            case 'd':
                config.decomposer_path = optarg;
                break;
            case 'p':
                if (std::string(optarg) == "full")
                    config.presolver_type = DSHunter::PresolverType::Full;
                else if (std::string(optarg) == "cheap")
                    config.presolver_type = DSHunter::PresolverType::Cheap;
                else if (std::string(optarg) == "none")
                    config.presolver_type = DSHunter::PresolverType::None;
                else
                    throw std::logic_error(std::string(optarg) +
                                           " is not a valid --presolve value");
                break;
            case 'm':
                if (std::string(optarg) == "ds_size")
                    mode = SOLUTION_SIZE;
                else if (std::string(optarg) == "presolve")
                    mode = PRESOLUTION;
                else if (std::string(optarg) == "treewidth")
                    mode = TREEWIDTH;
                else if (std::string(optarg) == "histogram")
                    mode = HISTOGRAM;
                else
                    throw std::logic_error(std::string(optarg) + " is not a valid --mode value");
                break;
            case 'h':
                print_help();
                break;
            default:
                print_help();
        }
    }
}

std::unique_ptr<std::istream> get_input_stream(const std::string& input_file) {
    if (!input_file.empty()) {
        auto input_stream = std::make_unique<std::ifstream>(input_file);
        if (!input_stream->is_open()) {
            std::cerr << "Error opening input file." << std::endl;
            exit(EXIT_FAILURE);
        }
        return input_stream;
    }
    return std::make_unique<std::istream>(std::cin.rdbuf());
}

std::unique_ptr<std::ostream> get_output_stream(const std::string& output_file) {
    if (!output_file.empty()) {
        auto output_stream = std::make_unique<std::ofstream>(output_file);
        if (!output_stream->is_open()) {
            std::cerr << "Error opening output file." << std::endl;
            exit(EXIT_FAILURE);
        }
        return output_stream;
    }
    return std::make_unique<std::ostream>(std::cout.rdbuf());
}

// .ads format description:
//  first line is 'p ads n m d' where:
//      n is the number of remaining nodes
//      m is the number of remaining edges
//      d is the number of nodes already known to be in the optimal dominating set
//      (those nodes are guaranteed to not be present in the graph)
//  second line is v_1, v_2, ..., v_d, being the list of nodes already known to be in the optimal
//  dominating set following it are n lines describing the nodes of the remaining graph in format 'v
//  s_d s_m e', where:
//      v is the node number in the original graph, note nodes may not be numbered from 1 to n.
//      s_d is the nodes domination status, 0 means undominated, 1 means dominated
//      s_m is the nodes solution membership status, 0 means maybe, 1 means no, 2 means yes.
//      e describes whether the node is an node non-existent in the original graph added by
//      reductions
//          0 means original node,
//          1 means extra node.
// comments starting with c can be only at the beginning of the file.

void export_presolution(const DSHunter::Instance& g, std::ostream& output) {
    output << "p ads " << g.nodeCount() << " " << g.edgeCount() << " " << g.ds.size() << "\n";
    for (auto v : g.ds) output << v << " ";
    output << "\n";

    for (auto v : g.nodes) {
        output << v << " " << (int)g[v].domination_status << " " << (int)g[v].membership_status << " " << g[v].is_extra << "\n";
    }

    for (auto u : g.nodes) {
        for (auto [v, status] : g[u].adj) {
            if (u < v)
                output << u << " " << v << " " << (int)status << "\n";
        }
    }
}

void solve_and_output(DSHunter::SolverConfig& config, std::istream& input, std::ostream& output, SolverMode mode) {
    DSHunter::Instance g(input);
    DSHunter::Solver solver(config);

    if (mode == TREEWIDTH) {
        solver.presolve(g);
        DSHunter::TreewidthSolver ts(&config);
        auto decomposition = ts.decomposer->decompose(g).value();
        auto treewidth = decomposition.width;
        output << treewidth << std::endl;
        return;
    }

    if (mode == HISTOGRAM) {
        solver.presolve(g);
        DSHunter::TreewidthSolver ts(&config);
        auto td = ts.decomposer->decompose(g).value();

        int width = td.width;
        std::vector<int> counts(width + 1);
        for (int i = 0; i < td.size(); i++)
            ++counts[td.bag[i].size()];

        std::cout << width << "\n";
        for (int i = 0; i <= width; i++) {
            std::cout << i << " " << counts[i] << "\n";
        }
        return;
    }

    if (mode == PRESOLUTION) {
        solver.presolve(g);
        export_presolution(g, output);
        std::cerr << dbg(g.edgeCount()) << dbg(g.forcedEdgeCount()) << std::endl;
        return;
    }

    auto ds = solver.solve(g);

    output << ds.size() << "\n";
    if (mode == SOLUTION) {
        for (auto v : ds) {
            output << v << "\n";
        }
    }
}
}  // namespace
int main(int argc, char* argv[]) {
    std::string input_file, output_file;
    DSHunter::SolverConfig config;
    SolverMode mode = SOLUTION;

    parse_arguments(argc, argv, input_file, output_file, config, mode);

    auto input = get_input_stream(input_file);
    auto output = get_output_stream(output_file);

    solve_and_output(config, *input, *output, mode);
}
