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
    bool unite(int a, int b) {
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
    int u, v, w;
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

    auto buildTree = [&](const vector<vector<int>>& value) {
        vector<Edge> candidates;
        for (int i = 0; i < N; ++i) {
            for (int j = i + 1; j < N; ++j) {
                if (C[i][j] + B[i][j] >= W) {
                    candidates.push_back({i, j, value[i][j]});
                }
            }
        }

        sort(candidates.begin(), candidates.end(),
             [](const Edge& a, const Edge& b) {
                 return a.w > b.w;
             });

        DSU dsu(N);
        vector<Edge> tree;
        for (const Edge& e : candidates) {
            if (dsu.unite(e.u, e.v)) {
                tree.push_back(e);
            }
        }

        if ((int)tree.size() != N - 1) return vector<Edge>();

        vector<vector<pair<int, int>>> adj(N);
        for (const Edge& e : tree) {
            adj[e.u].push_back({e.v, e.w});
            adj[e.v].push_back({e.u, e.w});
        }

        const int INF = 1e9;
        for (int s = 0; s < N; ++s) {
            vector<int> best(N, -1);
            stack<pair<int, int>> st;
            st.push({s, INF});
            best[s] = INF;

            while (!st.empty()) {
                int u = st.top().first;
                int cur = st.top().second;
                st.pop();

                for (auto [v, w] : adj[u]) {
                    if (best[v] == -1) {
                        best[v] = min(cur, w);
                        st.push({v, best[v]});
                    }
                }
            }

            for (int t = s + 1; t < N; ++t) {
                if (best[t] != value[s][t]) return vector<Edge>();
            }
        }

        return tree;
    };

    vector<Edge> carTree = buildTree(C);
    vector<Edge> bikeTree = buildTree(B);

    if (carTree.empty() || bikeTree.empty()) {
        cout << "NO\n";
        return 0;
    }

    cout << carTree.size() + bikeTree.size() << '\n';

    // Car tree: car width = C, so bike width = W-C.
    for (const Edge& e : carTree) {
        cout << e.u << ' ' << e.v << ' ' << W - e.w << '\n';
    }

    // Bike tree: bike width = B.
    for (const Edge& e : bikeTree) {
        cout << e.u << ' ' << e.v << ' ' << e.w << '\n';
    }

    return 0;
}
