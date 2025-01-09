#include <bits/stdc++.h>
using namespace std;
static mt19937 rng;
int R(int a, int b) { return uniform_int_distribution<int>(a, b)(rng); }
int32_t main(int argc, char *argv[]) {
    assert(argc > 1);
    hash<string> h;

    rng.seed(h(argv[1]));
    cout << "p ds ";
    int n = R(1, 16);
    int m = R(0, (n * (n - 1) / 2));
    cout << n << " " << m << "\n";
    vector<pair<int, int>> v;
    for (int i = 1; i <= n; i++) {
        for (int j = i + 1; j <= n; j++) {
            v.emplace_back(i, j);
        }
    }

    shuffle(v.begin(), v.end(), rng);
    while ((int)v.size() > m) v.pop_back();
    for (auto [a, b] : v) {
        if (R(0, 1)) swap(a, b);
        cout << a << " " << b << "\n";
    }
}