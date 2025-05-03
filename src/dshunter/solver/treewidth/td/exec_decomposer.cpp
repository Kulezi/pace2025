#include "exec_decomposer.h"

#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "../../../utils.h"

namespace DSHunter {

// Attempts to run an external decomposer on input_graph within time_limit.
// On success returns a filled TreeDecomposition, otherwise logs via DS_TRACE and returns nullopt.
std::optional<TreeDecomposition> ExecDecomposer::decompose(const Instance& input_graph) {
    int stdin_pipe[2];
    int stdout_pipe[2];
    if (pipe(stdin_pipe) == -1 || pipe(stdout_pipe) == -1) {
        DS_TRACE(std::cerr << "execDecompose: pipe() failed: " << strerror(errno) << std::endl);
        return std::nullopt;
    }

    pid_t pid = fork();
    if (pid == -1) {
        DS_TRACE(std::cerr << "execDecompose: fork() failed: " << strerror(errno) << std::endl);
        return std::nullopt;
    }

    if (pid == 0) {
        // Child: redirect stdin/stdout
        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        // Suppress stderr
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull != -1) {
            dup2(devnull, STDERR_FILENO);
            close(devnull);
        }
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        execl(cfg->decomposer_path.c_str(), cfg->decomposer_path.c_str(), static_cast<char*>(nullptr));
        _exit(1);
    }

    // Parent: close unused ends
    close(stdin_pipe[0]);
    close(stdout_pipe[1]);

    // Feed input_graph in PACE .gr simple format
    {
        std::vector<int> rv(input_graph.all_nodes.size());
        for (size_t i = 0; i < input_graph.nodes.size(); ++i) {
            rv[input_graph.nodes[i]] = static_cast<int>(i);
        }
        std::ostringstream oss;
        oss << "p tw " << input_graph.nodeCount() << " " << input_graph.edgeCount() << "\n";
        for (auto u : input_graph.nodes) {
            for (auto v: input_graph[u].n_open) {
                if (u > v)
                    continue;
                oss << rv[u] + 1 << " " << rv[v] + 1 << "\n";
            }
        }
        const std::string& data = oss.str();
        if (write(stdin_pipe[1], data.data(), data.size()) == -1) {
            DS_TRACE(std::cerr << "execDecompose: write() failed: " << strerror(errno) << std::endl);
        }
        close(stdin_pipe[1]);
    }

    // Capture output
    std::string output;
    std::thread reader([&]() {
        char buf[4096];
        ssize_t n;
        while ((n = read(stdout_pipe[0], buf, sizeof(buf))) > 0) {
            output.append(buf, n);
        }
    });

    // Wait with timeout
    auto start = std::chrono::steady_clock::now();
    int status = 0;
    while (true) {
        pid_t ret = waitpid(pid, &status, WNOHANG);
        if (ret == pid || ret == -1) {
            break;
        }
        if (std::chrono::steady_clock::now() - start > cfg->decomposition_time_budget) {
            kill(pid, SIGTERM);
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    reader.join();
    close(stdout_pipe[0]);

    // Parse .td output per Appendix B
    TreeDecomposition td;
    std::istringstream iss(output);
    std::string line;
    int N = 0, width_p1 = 0, num_vertices = 0;
    bool header_found = false;
    // Read solution line
    while (std::getline(iss, line)) {
        if (line.empty())
            continue;
        if (line[0] == 'c') {
            std::cerr << "execDecompose: " << line << std::endl;
            continue;
        }
        std::istringstream lss(line);
        char prefix;
        std::string tag;
        if (!(lss >> prefix >> tag >> N >> width_p1 >> num_vertices) || prefix != 's' || tag != "td") {
            DS_TRACE(std::cerr << "execDecompose: invalid or missing s-line: " << line << std::endl);
            return std::nullopt;
        }
        if (N <= 0) {
            // No decomposition found
            return std::nullopt;
        }
        td.width = width_p1;
        if (td.width > 20) {
            // width too large
            return std::nullopt;
        }
        header_found = true;
        td.bag.assign(N, {});
        td.adj.assign(N, {});
        break;
    }
    if (!header_found) {
        // No s-line at all: no decomposition
        return std::nullopt;
    }
    // Read bags and edges
    while (std::getline(iss, line)) {
        if (line.empty() || line[0] == 'c')
            continue;
        std::istringstream lss(line);
        char prefix;
        lss >> prefix;
        if (prefix == 'b') {
            int idx;
            if (!(lss >> idx) || idx < 1 || idx > N) {
                DS_TRACE(std::cerr << "execDecompose: invalid b-line index: " << line << std::endl);
                return std::nullopt;
            }
            std::vector<int> bag;
            int v;
            while (lss >> v) bag.push_back(input_graph.nodes[v - 1]);
            td.bag[idx - 1] = std::move(bag);
        } else {
            int u, v;
            std::istringstream ess(line);
            if (!(ess >> u >> v) || u < 1 || v < 1 || u > N || v > N) {
                DS_TRACE(std::cerr << "execDecompose: invalid edge line: " << line << std::endl);
                return std::nullopt;
            }
            td.addEdge(u - 1, v - 1);
        }
    }

    return td;
}

}  // namespace DSHunter
