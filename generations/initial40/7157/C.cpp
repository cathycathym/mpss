#include <bits/stdc++.h>
using namespace std;

struct DSU {
    vector<int> p, sz;
    DSU(int n) : p(n), sz(n, 1) {
        iota(p.begin(), p.end(), 0);
    }
    int find(int x) {
        return p[x] == x ? x : p[x] = find(p[x]);
    }
    bool join(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return false;
        if (sz[a] < sz[b]) swap(a, b);
        p[b] = a;
        sz[a] += sz[b];
        return true;
    }
};

struct Edge {
    int u, v, c, b;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, W;
    cin >> N >> W;

    vector<vector<int>> C(N, vector<int>(N, 0));
    vector<vector<int>> B(N, vector<int>(N, 0));

    for (int j = 1; j < N; ++j) {
        for (int i = 0; i < j; ++i) {
            cin >> C[i][j];
            C[j][i] = C[i][j];
        }
    }

    for (int j = 1; j < N; ++j) {
        for (int i = 0; i < j; ++i) {
            cin >> B[i][j];
            B[j][i] = B[i][j];
        }
    }

    vector<Edge> eligible;
    for (int i = 0; i < N; ++i) {
        for (int j = i + 1; j < N; ++j) {
            if (C[i][j] + B[i][j] >= W) {
                eligible.push_back({i, j, C[i][j], B[i][j]});
            }
        }
    }

    vector<tuple<int, int, int>> answer; // u, v, bike width

    vector<Edge> byCar = eligible;
    sort(byCar.begin(), byCar.end(), [](const Edge& x, const Edge& y) {
        return x.c > y.c;
    });

    DSU carDSU(N);
    for (const Edge& e : byCar) {
        if (carDSU.join(e.u, e.v)) {
            answer.emplace_back(e.u, e.v, W - e.c);
        }
    }

    vector<Edge> byBike = eligible;
    sort(byBike.begin(), byBike.end(), [](const Edge& x, const Edge& y) {
        return x.b > y.b;
    });

    DSU bikeDSU(N);
    for (const Edge& e : byBike) {
        if (bikeDSU.join(e.u, e.v)) {
            answer.emplace_back(e.u, e.v, e.b);
        }
    }

    vector<vector<int>> realC(N, vector<int>(N, 0));
    vector<vector<int>> realB(N, vector<int>(N, 0));

    for (int i = 0; i < N; ++i) {
        realC[i][i] = W;
        realB[i][i] = W;
    }

    for (auto [u, v, b] : answer) {
        int c = W - b;
        realC[u][v] = realC[v][u] = max(realC[u][v], c);
        realB[u][v] = realB[v][u] = max(realB[u][v], b);
    }

    for (int k = 0; k < N; ++k) {
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                realC[i][j] = max(realC[i][j], min(realC[i][k], realC[k][j]));
                realB[i][j] = max(realB[i][j], min(realB[i][k], realB[k][j]));
            }
        }
    }

    for (int i = 0; i < N; ++i) {
        for (int j = i + 1; j < N; ++j) {
            if (realC[i][j] != C[i][j] || realB[i][j] != B[i][j]) {
                cout << "NO\n";
                return 0;
            }
        }
    }

    cout << answer.size() << '\n';
    for (auto [u, v, b] : answer) {
        cout << u << ' ' << v << ' ' << b << '\n';
    }

    return 0;
}
