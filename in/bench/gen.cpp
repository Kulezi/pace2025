#include <bits/stdc++.h>
using namespace std;
using Edges = vector<pair<int, int>>;

void gen_star(int l, int r, Edges &e) {
    for (int i = l + 1; i <= r; i++) {
        e.emplace_back(l, i);
    }
}

void gen_path(int l, int r, Edges &e) {
    for (int i = l + 1; i <= r; i++) {
        e.emplace_back(i - 1, i);
    }
}

void gen_clique(int l, int r, Edges &e) {
    for (int i = l; i <= r; i++) {
        for (int j = i + 1; j <= r; j++) {
            e.emplace_back(i, j);
        }
    }
}

void gen_grid(int n, int m, int offset, Edges &e) {
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= m; j++) {
            int l = j + (i - 1) * m + offset;
            if (i + 1 <= n) {
                e.emplace_back(l, l + m);
            }
            if (j + 1 <= m) {
                e.emplace_back(l, l + 1);
            }
        }
    }
}

static mt19937 rng(0);
int R(int a, int b) { return uniform_int_distribution<int>(a, b)(rng); }

void gen_random_tree(int l, int r, Edges &e) {
    for (int i = l + 1; i <= r; i++) {
        e.emplace_back(R(l, i - 1), i);
    }
}

void gen_random_edges(int l, int r, Edges &e) {
    Edges to_draw;
    for (int i = l; i <= r; i++) {
        for (int j = i + 1; j <= r; j++) {
            to_draw.emplace_back(i, j);
        }
    }

    shuffle(to_draw.begin(), to_draw.end(), rng);
    to_draw.resize(R(0, to_draw.size()));
    for (auto edge : to_draw) e.push_back(edge);
}

void make_tc(string name, Edges e) {
    ofstream out(name + ".gr");

    int n = 0;
    for (auto &[l, r] : e) n = max({n, l, r});
    out << "c " << name << "\n";
    out << "p ds " << n << " " << e.size() << "\n";
    for (auto &[l, r] : e) {
        out << l << " " << r << "\n";
    }
}

int32_t main() {
    int N_LINEAR = 1000;
    make_tc("star" + to_string(N_LINEAR), [&]() -> Edges {
        Edges res;
        gen_star(1, N_LINEAR, res);
        return res;
    }());

    make_tc("path" + to_string(N_LINEAR), [&]() -> Edges {
        Edges res;
        gen_path(1, N_LINEAR, res);
        return res;
    }());

    make_tc("tree" + to_string(N_LINEAR), [&]() -> Edges {
        Edges res;
        gen_random_tree(1, N_LINEAR, res);
        return res;
    }());

    int MAX_CLIQUE = 20;
    make_tc("cliques" + to_string(MAX_CLIQUE), [&]() -> Edges {
        Edges res;
        int start = 1;
        for (int i = 1; i <= MAX_CLIQUE; i++) {
            gen_clique(start, start + i - 1, res);
            start += i;
        }
        return res;
    }());

    vector<int> grid_ns = {2, 3, 5, 7, 10};
    for (auto n : grid_ns) {
        int m = 50 / n;
        string tc = "grid_" + to_string(n) + "_" + to_string(m);
        make_tc(tc, [&]() -> Edges {
            Edges res;
            gen_grid(n, m, 0, res);
            return res;
        }());
    }

    constexpr int N_RANDOM_GRAPHS = 300, MAX_SIZE = 16;
    make_tc("random_small_components_x" + to_string(N_RANDOM_GRAPHS), [&]() -> Edges {
        Edges e;
        int start = 1;
        for (int i = 0; i < N_RANDOM_GRAPHS; i++) {
            int n = R(1, MAX_SIZE);

            gen_random_edges(start, start + n - 1, e);
            start += n;
        }
        return e;
    }());
}