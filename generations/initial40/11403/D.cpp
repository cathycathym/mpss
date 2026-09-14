#include <bits/stdc++.h>
using namespace std;

struct DSU {
    vector<int> parent, size;

    explicit DSU(int n) : parent(n), size(n, 1) {
        iota(parent.begin(), parent.end(), 0);
    }

    int find(int x) {
        return parent[x] == x ? x : parent[x] = find(parent[x]);
    }

    int unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return a;
        if (size[a] < size[b]) swap(a, b);
        parent[b] = a;
        size[a] += size[b];
        return a;
    }
};

struct Edge {
    int u, v, weight;

    bool operator<(const Edge& other) const {
        return weight < other.weight;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int H, W;
    long long L;
    cin >> H >> W >> L;

    const int N = H * W;
    vector<int> height(N);
    for (int& x : height) cin >> x;

    vector<Edge> edges;
    edges.reserve(2 * N);

    for (int r = 0; r < H; ++r) {
        for (int c = 0; c < W; ++c) {
            int u = r * W + c;

            if (r + 1 < H) {
                int v = u + W;
                edges.push_back({u, v, max(height[u], height[v])});
            }
            if (c + 1 < W) {
                int v = u + 1;
                edges.push_back({u, v, max(height[u], height[v])});
            }
        }
    }

    sort(edges.begin(), edges.end());

    const int maxNodes = 2 * N;
    vector<int> treeParent(maxNodes, -1);
    vector<int> leftChild(maxNodes, -1);
    vector<int> rightChild(maxNodes, -1);
    vector<int> value(maxNodes);

    for (int i = 0; i < N; ++i) value[i] = height[i];

    DSU dsu(N);
    vector<int> componentRoot(N);
    iota(componentRoot.begin(), componentRoot.end(), 0);

    int nodes = N;

    for (const Edge& edge : edges) {
        int a = dsu.find(edge.u);
        int b = dsu.find(edge.v);
        if (a == b) continue;

        int x = componentRoot[a];
        int y = componentRoot[b];
        int z = nodes++;

        value[z] = edge.weight;
        leftChild[z] = x;
        rightChild[z] = y;
        treeParent[x] = z;
        treeParent[y] = z;

        int merged = dsu.unite(a, b);
        componentRoot[merged] = z;
    }

    const int root = componentRoot[dsu.find(0)];

    vector<int> tin(nodes), tout(nodes);
    int timer = 0;

    vector<pair<int, bool>> stack;
    stack.reserve(2 * nodes);
    stack.push_back({root, false});

    while (!stack.empty()) {
        auto [v, exiting] = stack.back();
        stack.pop_back();

        if (!exiting) {
            tin[v] = timer++;
            stack.push_back({v, true});

            if (rightChild[v] != -1)
                stack.push_back({rightChild[v], false});
            if (leftChild[v] != -1)
                stack.push_back({leftChild[v], false});
        } else {
            tout[v] = timer - 1;
        }
    }

    auto contains = [&](int component, int vertex) {
        return tin[component] <= tin[vertex] &&
               tout[vertex] <= tout[component];
    };

    int LOG = 1;
    while ((1LL << (LOG - 1)) < nodes) ++LOG;

    vector<vector<int>> up(LOG, vector<int>(nodes, -1));
    for (int v = 0; v < nodes; ++v)
        up[0][v] = treeParent[v];

    for (int k = 1; k < LOG; ++k) {
        for (int v = 0; v < nodes; ++v) {
            int middle = up[k - 1][v];
            if (middle != -1)
                up[k][v] = up[k - 1][middle];
        }
    }

    vector<vector<int>> jump(LOG, vector<int>(nodes));

    for (int v = 0; v < nodes; ++v) {
        long long limit = static_cast<long long>(value[v]) + L;
        int current = v;

        for (int k = LOG - 1; k >= 0; --k) {
            int ancestor = up[k][current];
            if (ancestor != -1 &&
                static_cast<long long>(value[ancestor]) <= limit) {
                current = ancestor;
            }
        }

        jump[0][v] = current;
    }

    for (int k = 1; k < LOG; ++k) {
        for (int v = 0; v < nodes; ++v) {
            jump[k][v] = jump[k - 1][jump[k - 1][v]];
        }
    }

    int Q;
    cin >> Q;

    while (Q--) {
        int A, B, C, D;
        cin >> A >> B >> C >> D;

        int source = (A - 1) * W + (B - 1);
        int target = (C - 1) * W + (D - 1);

        int eventual = jump[LOG - 1][source];
        if (!contains(eventual, target)) {
            cout << -1 << '\n';
            continue;
        }

        int current = source;
        int answer = 0;

        for (int k = LOG - 1; k >= 0; --k) {
            int next = jump[k][current];
            if (!contains(next, target)) {
                current = next;
                answer += (1 << k);
            }
        }

        cout << answer + 1 << '\n';
    }

    return 0;
}