#include <bits/stdc++.h>
using namespace std;

struct Edge {
    int u, v, w;
    bool operator<(const Edge& other) const {
        return w < other.w;
    }
};

struct DSU {
    vector<int> p, sz, best;
    const vector<int>* h;

    DSU(int n, const vector<int>& heights) : p(n), sz(n, 1), best(n), h(&heights) {
        iota(p.begin(), p.end(), 0);
        iota(best.begin(), best.end(), 0);
    }

    int find(int x) {
        while (p[x] != x) {
            p[x] = p[p[x]];
            x = p[x];
        }
        return x;
    }

    void unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return;
        if (sz[a] < sz[b]) swap(a, b);
        p[b] = a;
        sz[a] += sz[b];

        int x = best[a], y = best[b];
        if ((*h)[y] > (*h)[x] || ((*h)[y] == (*h)[x] && y < x))
            best[a] = y;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int H, W;
    long long L;
    cin >> H >> W >> L;

    int N = H * W;
    vector<int> height(N);
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
                int v = r * W + c + 1;
                edges.push_back({u, v, max(height[u], height[v])});
            }
        }
    }

    // Kruskal reconstruction tree:
    // value[LCA(u,v)] = minimum possible maximum mountain height on a path u-v.
    sort(edges.begin(), edges.end());

    int MAXT = 2 * N;
    vector<int> dsuTree(MAXT), treeParent(MAXT, -1);
    vector<int> leftChild(MAXT, -1), rightChild(MAXT, -1);
    vector<int> value(MAXT);

    for (int i = 0; i < N; ++i) {
        dsuTree[i] = i;
        value[i] = height[i];
    }

    function<int(int)> findTree = [&](int x) {
        if (dsuTree[x] == x) return x;
        return dsuTree[x] = findTree(dsuTree[x]);
    };

    int total = N;
    for (const Edge& e : edges) {
        int a = findTree(e.u);
        int b = findTree(e.v);
        if (a == b) continue;

        int z = total++;
        value[z] = e.w;
        leftChild[z] = a;
        rightChild[z] = b;
        treeParent[a] = z;
        treeParent[b] = z;
        dsuTree[a] = z;
        dsuTree[b] = z;
        dsuTree[z] = z;
    }

    int rootTree = findTree(0);

    int LOGT = 1;
    while ((1 << LOGT) <= total) ++LOGT;

    vector<vector<int>> upTree(LOGT, vector<int>(total));
    vector<int> depth(total, 0);

    for (int i = 0; i < total; ++i) {
        upTree[0][i] = (treeParent[i] == -1 ? i : treeParent[i]);
    }

    vector<int> st;
    st.push_back(rootTree);
    while (!st.empty()) {
        int x = st.back();
        st.pop_back();
        if (leftChild[x] != -1) {
            int a = leftChild[x], b = rightChild[x];
            depth[a] = depth[x] + 1;
            depth[b] = depth[x] + 1;
            st.push_back(a);
            st.push_back(b);
        }
    }

    for (int k = 1; k < LOGT; ++k) {
        for (int i = 0; i < total; ++i) {
            upTree[k][i] = upTree[k - 1][upTree[k - 1][i]];
        }
    }

    auto lca = [&](int a, int b) {
        if (depth[a] < depth[b]) swap(a, b);
        int diff = depth[a] - depth[b];
        for (int k = 0; k < LOGT; ++k) {
            if (diff & (1 << k)) a = upTree[k][a];
        }
        if (a == b) return a;
        for (int k = LOGT - 1; k >= 0; --k) {
            if (upTree[k][a] != upTree[k][b]) {
                a = upTree[k][a];
                b = upTree[k][b];
            }
        }
        return upTree[0][a];
    };

    // For each mountain u, find the highest mountain reachable in one jump.
    vector<int> order(N);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        return height[a] < height[b];
    });

    vector<int> thresholdOrder(N);
    iota(thresholdOrder.begin(), thresholdOrder.end(), 0);
    sort(thresholdOrder.begin(), thresholdOrder.end(), [&](int a, int b) {
        return (long long)height[a] + L < (long long)height[b] + L;
    });

    DSU sweep(N, height);
    vector<char> active(N, 0);
    vector<int> nextBest(N);

    int ptr = 0;
    for (int u : thresholdOrder) {
        long long limit = (long long)height[u] + L;

        while (ptr < N && height[order[ptr]] <= limit) {
            int x = order[ptr++];
            active[x] = 1;

            int r = x / W, c = x % W;
            if (r > 0) {
                int y = x - W;
                if (active[y]) sweep.unite(x, y);
            }
            if (r + 1 < H) {
                int y = x + W;
                if (active[y]) sweep.unite(x, y);
            }
            if (c > 0) {
                int y = x - 1;
                if (active[y]) sweep.unite(x, y);
            }
            if (c + 1 < W) {
                int y = x + 1;
                if (active[y]) sweep.unite(x, y);
            }
        }

        int b = sweep.best[sweep.find(u)];
        // Equal-height moves do not improve future jumping power.
        nextBest[u] = (height[b] > height[u] ? b : u);
    }

    int LOGN = 1;
    while ((1 << LOGN) <= N) ++LOGN;

    vector<vector<int>> jump(LOGN, vector<int>(N));
    jump[0] = nextBest;
    for (int k = 1; k < LOGN; ++k) {
        for (int i = 0; i < N; ++i) {
            jump[k][i] = jump[k - 1][jump[k - 1][i]];
        }
    }

    int Q;
    cin >> Q;
    while (Q--) {
        int A, B, C, D;
        cin >> A >> B >> C >> D;
        --A; --B; --C; --D;

        int s = A * W + B;
        int t = C * W + D;

        // This is the least altitude threshold required to connect s and t.
        long long bottleneck = value[lca(s, t)];
        long long neededHeight = bottleneck - L;

        int highest = s;
        for (int k = LOGN - 1; k >= 0; --k) {
            highest = jump[k][highest];
        }

        if ((long long)height[highest] < neededHeight) {
            cout << -1 << '\n';
            continue;
        }

        // Find the last state whose height is still insufficient.
        int cur = s;
        int steps = 0;
        for (int k = LOGN - 1; k >= 0; --k) {
            int nx = jump[k][cur];
            if ((long long)height[nx] < neededHeight) {
                cur = nx;
                steps += (1 << k);
            }
        }

        // One final jump from cur reaches the destination.
        cout << steps + 1 << '\n';
    }

    return 0;
}
