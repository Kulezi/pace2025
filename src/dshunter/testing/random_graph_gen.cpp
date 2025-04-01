#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
using namespace std;
constexpr int MIN_NODES = 1, MAX_NODES = 13;

int32_t main() {
    mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
    auto R = [&](int a, int b) { return uniform_int_distribution<int>(a, b)(rng); };
    int n = R(MIN_NODES, MAX_NODES);
    int m = R(0, (n * (n - 1) / 2));

    cout << "p ds " << n << " " << m << "\n";

    vector<pair<int, int>> v;
    for (int i = 1; i <= n; i++) {
        for (int j = i + 1; j <= n; j++) {
            v.emplace_back(i, j);
        }
    }

    shuffle(v.begin(), v.end(), rng);
    v.resize(m);
    for (auto [a, b] : v) {
        if (R(0, 1)) swap(a, b);
        cout << a << " " << b << "\n";
    }
}