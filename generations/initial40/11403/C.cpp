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
    int unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return a;
        if (sz[a] < sz[b]) swap(a, b);
        p[b] = a;
        sz[a] += sz[b];
        return a;
    }
};

struct Edge {
    int u, v;
    long long w;
    bool operator<(const Edge& other) const {
        return w < other.w;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int H, W;
    long long L;
    cin >> H >> W >> L;
    int N = H * W;

    vector<long long> height(N);
    for (int i = 0; i < N; ++i) cin >> height[i];

    vector<Edge> edges;
    edges.reserve(2 * N);
    for (int r = 0; r < H; ++r) {
        for (int c = 0; c < W; ++c) {
            int u = r * W + c;
            if (r + 1 < H) {
                int v = (r + 1) * W + c;
                edges.push_back({u, v, max(height[u], height[v])});
            }
            if (c + 1 < W) {
                int v = r * W + (c + 1);
                edges.push_back({u, v, max(height[u], height[v])});
            }
        }
    }
    sort(edges.begin(), edges.end());

    int MAXV = 2 * N;
    vector<int> parent(MAXV, -1), leftChild(MAXV, -1), rightChild(MAXV, -1);
    vector<long long> value(MAXV), maximumHeight(MAXV);

    for (int i = 0; i < N; ++i) {
        value[i] = maximumHeight[i] = height[i];
    }

    DSU dsu(N);
    vector<int> treeRoot(N);
    iota(treeRoot.begin(), treeRoot.end(), 0);

    int nodes = N;
    for (const Edge& e : edges) {
        int a = dsu.find(e.u);
        int b = dsu.find(e.v);
        if (a == b) continue;

        int x = treeRoot[a];
        int y = treeRoot[b];
        int z = nodes++;

        value[z] = e.w;
        maximumHeight[z] = max(maximumHeight[x], maximumHeight[y]);
        leftChild[z] = x;
        rightChild[z] = y;
        parent[x] = parent[y] = z;

        int root = dsu.unite(a, b);
        treeRoot[root] = z;
    }

    int root = treeRoot[dsu.find(0)];

    vector<int> tin(nodes), tout(nodes);
    int timer = 0;
    vector<pair<int, int>> st;
    st.push_back({root, 0});
    while (!st.empty()) {
        int v = st.back().first;
        int state = st.back().second;
        st.pop_back();

        if (state == 0) {
            tin[v] = timer++;
            st.push_back({v, 1});
            if (rightChild[v] != -1) st.push_back({rightChild[v], 0});
            if (leftChild[v] != -1) st.push_back({leftChild[v], 0});
        } else {
            tout[v] = timer - 1;
        }
    }

    int LOG = 1;
    while ((1LL << (LOG - 1)) < nodes) ++LOG;

    vector<vector<int>> up(LOG, vector<int>(nodes, -1));
    for (int v = 0; v < nodes; ++v) up[0][v] = parent[v];
    for (int j = 1; j < LOG; ++j) {
        for (int v = 0; v < nodes; ++v) {
            int mid = up[j - 1][v];
            up[j][v] = (mid == -1 ? -1 : up[j - 1][mid]);
        }
    }

    vector<int> jump0(nodes);
    for (int v = 0; v < nodes; ++v) {
        long long limit = maximumHeight[v] + L;
        int cur = v;
        for (int j = LOG - 1; j >= 0; --j) {
            int anc = up[j][cur];
            if (anc != -1 && value[anc] <= limit) cur = anc;
        }
        jump0[v] = cur;
    }

    vector<vector<int>> jump(LOG, vector<int>(nodes));
    jump[0] = jump0;
    for (int j = 1; j < LOG; ++j) {
        for (int v = 0; v < nodes; ++v) {
            jump[j][v] = jump[j - 1][jump[j - 1][v]];
        }
    }

    auto isAncestor = [&](int a, int b) {
        return tin[a] <= tin[b] && tout[b] <= tout[a];
    };

    int Q;
    cin >> Q;
    while (Q--) {
        int A, B, C, D;
        cin >> A >> B >> C >> D;
        int source = (A - 1) * W + (B - 1);
        int target = (C - 1) * W + (D - 1);

        int finalComponent = jump[LOG - 1][source];
        if (!isAncestor(finalComponent, target)) {
            cout << -1 << '\n';
            continue;
        }

        int cur = source;
        int answer = 0;
        for (int j = LOG - 1; j >= 0; --j) {
            int next = jump[j][cur];
            if (!isAncestor(next, target)) {
                cur = next;
                answer += (1 << j);
            }
        }
        cout << answer + 1 << '\n';
    }

    return 0;
}
