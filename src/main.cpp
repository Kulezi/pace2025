#include <getopt.h>

#include <fstream>
#include <iostream>
#include <memory>

#include "dshunter/dshunter.h"

void print_help() {
    std::cout
        << "Usage:\n"
        << "  dshunter [--input_file <graph.gr>] "
        << "[--output_file <file.ds>] "
        << "[--method <bruteforce/branching/treewidth_dp/mip/vc/gurobi>] "
        << "[--presolve <full/cheap/none>] [--short] [--help]\n\n"

        << "Options:\n"
        << "  --input_file    Read instance from specified file (default: stdin)\n"
        << "  --output_file   Write solution to specified file (default: stdout)\n"
        << "  --method        Choose solving method: bruteforce, branching, treewidth_dp, "
           "mip, vc, gurobi\n"
        << "  --presolve      Choose presolver: full, cheap, none\n"
        << "  --short         Output only the solution set size\n"
        << "  --help          Show this help message and exit\n\n"

        << "By default dshunter reads the instance in .gr format from stdin.\n"
        << "--input_file flag overrides this behaviour making it read from a file.\n\n"

        << "To force dshunter to use a certain method of solving the instance\n"
        << "--method flag can be used with its respective value.\n\n"

        << "By default dshunter will decide by itself whether to presolve the instance or not.\n"
        << "--presolve flag can be used to force certain presolver behaviour.\n\n"
        
        << "By default dshunter will print full solution in PACE2025 Dominating Set solution "
           "format.\n"
        << "--short flag makes dshunter output only the solution set size.\n\n";
    exit(EXIT_SUCCESS);
}

void parse_arguments(int argc, char* argv[], std::string& input_file, std::string& output_file,
                     DSHunter::SolverConfig& config, bool& print_solution_set) {
    struct option long_options[] = {{"input_file", required_argument, nullptr, 'i'},
                                    {"output_file", required_argument, nullptr, 'o'},
                                    {"method", required_argument, nullptr, 'm'},
                                    {"presolve", required_argument, nullptr, 'p'},
                                    {"short", no_argument, nullptr, 's'},
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
            case 'm':
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
                break;
            case 'p':
                if (std::string(optarg) == "full")
                    config.presolver_type = DSHunter::PresolverType::Full;
                else if (std::string(optarg) == "cheap")
                    config.presolver_type = DSHunter::PresolverType::Cheap;
                else if (std::string(optarg) == "none")
                    config.presolver_type = DSHunter::PresolverType::None;
                break;
            case 's':
                print_solution_set = false;
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

void solve_and_output(DSHunter::SolverConfig& config, std::istream& input, std::ostream& output,
                      bool print_solution_set) {
    DSHunter::Instance g(input);
    DSHunter::Solver solver(config);
    auto ds = solver.solve(g);

    output << ds.size() << "\n";
    if (print_solution_set) {
        for (auto v : ds) {
            output << v << "\n";
        }
    }
}

int main(int argc, char* argv[]) {
    std::string input_file;
    std::string output_file;
    DSHunter::SolverConfig config;
    bool print_solution_set = true;

    parse_arguments(argc, argv, input_file, output_file, config, print_solution_set);

    auto input = get_input_stream(input_file);
    auto output = get_output_stream(output_file);

    solve_and_output(config, *input, *output, print_solution_set);
}
