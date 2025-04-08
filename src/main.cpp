#include <getopt.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>

#include "dshunter/dshunter.h"
#include "dshunter/solver/treewidth/td/flow_cutter_wrapper.h"
namespace {
void print_help() {
    std::cout
        << "Usage:\n"
        << "  dshunter [--input_file <graph.gr/graph.ads>] "
        << "[--output_file <file.ds>] "
        << "[--solver <bruteforce/branching/treewidth_dp/mip/vc/gurobi>] "
        << "[--export] <graph.ads>"
        << "[--mode] <presolve/ds_size/treewidth>"
        << "[--presolve <full/cheap/none>] [--short] [--help]\n\n"

        << "Options:\n"
        << "  --input_file    Read instance from specified file (default: stdin)\n"
        << "  --output_file   Write solution to specified file (default: stdout)\n"
        << "  --solver        Choose solving method: bruteforce, branching, treewidth_dp, "
           "mip, vc, gurobi\n"
        << "  --export        Export the presolved instance to file\n"
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
           "presolving.\n\n";
    exit(EXIT_SUCCESS);
}

enum SolverMode {
    SOLUTION,
    PRESOLUTION,
    SOLUTION_SIZE,
    TREEWIDTH,
};

void parse_arguments(int argc, char* argv[], std::string& input_file, std::string& output_file,
                     DSHunter::SolverConfig& config, SolverMode& mode) {
    struct option long_options[] = {{"input_file", required_argument, nullptr, 'i'},
                                    {"output_file", required_argument, nullptr, 'o'},
                                    {"solver", required_argument, nullptr, 's'},
                                    {"mode", required_argument, nullptr, 'm'},
                                    {"presolve", required_argument, nullptr, 'p'},
                                    {"help", no_argument, nullptr, 'h'},
                                    {nullptr, 0, nullptr, 0}};

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
//  s e', where:
//      v is the node number in the original graph, note nodes may not be numbered from 1 to n.
//      s is the nodes status, 0 means undominated, 1 means dominated
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
        output << v << " " << g.getNodeStatus(v) << " " << g.is_extra[v] << "\n";
    }

    for (auto u : g.nodes) {
        for (auto [v, status] : g.adj[u]) {
            if (u < v) output << u << " " << v << " " << (int)status << "\n";
        }
    }
}

void solve_and_output(DSHunter::SolverConfig& config, std::istream& input, std::ostream& output,
                      SolverMode mode) {
    DSHunter::Instance g(input);
    DSHunter::Solver solver(config);

    if (mode == TREEWIDTH) {
        solver.presolve(g);
        auto decomposition = DSHunter::FlowCutter::decompose(g, 0, std::chrono::seconds(60), 15);
        auto treewidth = decomposition.width;
        output << treewidth << std::endl;
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
