#include <iostream>
#include <fstream>
#include <string>
#include "Highs.h"
#include "ds.h"
#include "instance.h"

using namespace std;

int32_t main(int argc, char *argv[]) {
    assert(argc >= 1);

    std::string filepath = argv[1];
    ifstream input(filepath);

    Instance g(input);

    int n = g.nodeCount();
    int f = g.forcedEdgeCount();

    HighsModel model;

    model.lp_.num_col_ = n;
    model.lp_.num_row_ = n + f;
    model.lp_.sense_ = ObjSense::kMinimize;
    model.lp_.offset_ = 0;
    model.lp_.col_cost_ = std::vector<double>(n, 1.0);
    model.lp_.col_lower_ = std::vector<double>(n, 0.0);
    model.lp_.col_upper_ = std::vector<double>(n, 1.0);
    model.lp_.row_lower_ = std::vector<double>(n + f, 1.0);
    model.lp_.row_upper_ = std::vector<double>(n + f, n);

    // Here the orientation of the matrix is column-wise
    model.lp_.a_matrix_.format_ = MatrixFormat::kRowwise;

    // a_start_ has num_col_+1 entries, and the last entry is the number
    // of nonzeros in A, allowing the number of nonzeros in the last
    // column to be defined

    vector<int> start, index;
    vector<double> value;

    vector<int> v(n), rv(g.nodes.back() + 1);
    for (int i = 0; i < n; i++) {
        v[i] = g.nodes[i];
        rv[g.nodes[i]] = i;
    }

    int tot = 0;
    for (int i = 0; i < n; i++) {
        start.push_back(tot);
        index.push_back(i);
        value.push_back(1);
        for (auto j : g.adj[v[i]]) {
            index.push_back(rv[j.to]);
            value.push_back(1);
        }

        tot += g.deg(v[i]) + 1;
    }

    for (int i = 0; i < n; i++) {
        for (auto [u, status] : g.adj[v[i]]) {
            // Don't repeat constraints.
            if (status == FORCED && u > v[i]) {
                start.push_back(tot);

                index.push_back(rv[u]);
                index.push_back(i);

                value.push_back(1);
                value.push_back(1);

                tot += 2;
            }
        }
    }

    start.push_back(tot);

    model.lp_.a_matrix_.start_ = start;
    model.lp_.a_matrix_.index_ = index;
    model.lp_.a_matrix_.value_ = value;

    model.lp_.integrality_.resize(model.lp_.num_col_);
    for (int col = 0; col < model.lp_.num_col_; col++)
        model.lp_.integrality_[col] = HighsVarType::kInteger;
    // Create a Highs instance
    Highs highs;
    HighsStatus return_status;

    // Pass the model to HiGHS
    return_status = highs.passModel(model);
    assert(return_status == HighsStatus::kOk);

    // Get a const reference to the LP data in HiGHS
    const HighsLp& lp = highs.getLp();

    highs.writeModel(std::string(argv[1]) + ".lp");

    // Solve the model
    // return_status = highs.run();
    // assert(return_status == HighsStatus::kOk);

    // // Get the model status
    // const HighsModelStatus& model_status = highs.getModelStatus();
    // assert(model_status == HighsModelStatus::kOptimal);

    // const HighsInfo& info = highs.getInfo();
    // cout << "Simplex iteration count: " << info.simplex_iteration_count << endl;
    // cout << "Objective function value: " << info.objective_function_value << endl;
    // cout << "Primal  solution status: " << highs.solutionStatusToString(info.primal_solution_status)
    //      << endl;
    // cout << "Dual    solution status: " << highs.solutionStatusToString(info.dual_solution_status)
    //      << endl;
    // cout << "Basis: " << highs.basisValidityToString(info.basis_validity) << endl;
}