#include <bits/stdc++.h>
using namespace std;
int R(int a, int b) {
    static mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
    return uniform_int_distribution<int>(a, b)(rng);
}
int32_t main() {
    srand(R(1, 123891213));
    cout << "p ds ";
    int n = R(1, 10);
    int m = R(1, (n * (n - 1) / 2));
    cout << n << " " << m << "\n";
    vector<pair<int,int>> v;
    for (int i = 1; i <= n; i++) {
        for (int j = i+1; j <= n; j++) {
            v.emplace_back(i, j);
        }
    }

    random_shuffle(v.begin(), v.end());
    while (v.size() > m) v.pop_back();
    for (auto [a, b] : v) {
        if (R(0, 1)) swap(a, b);
        cout << a << " " << b << "\n";
    }
}