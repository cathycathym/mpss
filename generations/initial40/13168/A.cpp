#include <bits/stdc++.h>
using namespace std;

using ll = long long;

struct Edge {
    int u, v;
    ll w;
    bool operator<(const Edge& other) const {
        return w < other.w;
    }
};

struct Query {
    int l, r, id;
    long long ord;
};

static long long hilbertOrder(int x, int y, int pw, int rot = 0) {
    if (pw == 0) return 0;
    int h = 1 << (pw - 1);
    int seg = (x < h) ? ((y < h) ? 0 : 3) : ((y < h) ? 1 : 2);
    seg = (seg + rot) & 3;

    static const int rotateDelta[4] = {3, 0, 0, 1};
    int nx = x & (h - 1);
    int ny = y & (h - 1);
    int nrot = (rot + rotateDelta[seg]) & 3;

    long long sub = hilbertOrder(nx, ny, pw - 1, nrot);
    long long block = 1LL << (2 * pw - 2);
    long long ans = seg * block;

    if (seg == 1 || seg == 2) return ans + sub;
    return ans + block - sub - 1;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M, Q;
    cin >> N >> M >> Q;

    vector<Edge> edges(M);
    for (auto &e : edges) cin >> e.u >> e.v >> e.w;
    sort(edges.begin(), edges.end());

    // Kruskal reconstruction tree:
    // leaves 0..N-1 are turbines, every merge creates one internal node.
    int maxNodes = 2 * N;
    vector<int> dsu(N), componentNode(N);
    iota(dsu.begin(), dsu.end(), 0);
    iota(componentNode.begin(), componentNode.end(), 0);

    auto findRoot = [&](int x) {
        int r = x;
        while (dsu[r] != r) r = dsu[r];
        while (dsu[x] != x) {
            int p = dsu[x];
            dsu[x] = r;
            x = p;
        }
        return r;
    };

    vector<int> leftChild(maxNodes, -1), rightChild(maxNodes, -1);
    vector<ll> value(maxNodes, 0);

    int nodes = N;
    ll mstWeight = 0;

    for (const auto &e : edges) {
        int a = findRoot(e.u);
        int b = findRoot(e.v);
        if (a == b) continue;

        int cur = nodes++;
        leftChild[cur] = componentNode[a];
        rightChild[cur] = componentNode[b];
        value[cur] = e.w;

        dsu[b] = a;
        componentNode[a] = cur;
        mstWeight += e.w;
    }

    int root = componentNode[findRoot(0)];

    // Iterative DFS: Euler tour for O(1) LCA and leaf DFS order.
    vector<int> depth(nodes, 0), first(nodes, -1), leafPos(N), leafAt(N);
    vector<int> euler;
    euler.reserve(2 * nodes);

    vector<pair<int, int>> stack;
    stack.push_back({root, 0});

    int leafCount = 0;
    while (!stack.empty()) {
        int u = stack.back().first;
        int state = stack.back().second;

        if (state == 0) {
            first[u] = (int)euler.size();
            euler.push_back(u);

            if (u < N) {
                leafPos[u] = leafCount;
                leafAt[leafCount] = u;
                ++leafCount;
                stack.pop_back();
            } else {
                stack.back().second = 1;
                int v = leftChild[u];
                depth[v] = depth[u] + 1;
                stack.push_back({v, 0});
            }
        } else if (state == 1) {
            euler.push_back(u);
            stack.back().second = 2;
            int v = rightChild[u];
            depth[v] = depth[u] + 1;
            stack.push_back({v, 0});
        } else {
            euler.push_back(u);
            stack.pop_back();
        }
    }

    int E = (int)euler.size();
    vector<int> lg(E + 1, 0);
    for (int i = 2; i <= E; ++i) lg[i] = lg[i / 2] + 1;

    int LOG = lg[E] + 1;
    vector<vector<int>> sparse(LOG, vector<int>(E));
    sparse[0] = euler;

    for (int k = 1; k < LOG; ++k) {
        int len = 1 << k;
        int half = len >> 1;
        for (int i = 0; i + len <= E; ++i) {
            int a = sparse[k - 1][i];
            int b = sparse[k - 1][i + half];
            sparse[k][i] = (depth[a] < depth[b] ? a : b);
        }
    }

    auto lca = [&](int a, int b) {
        int x = first[a], y = first[b];
        if (x > y) swap(x, y);
        int k = lg[y - x + 1];
        int p = sparse[k][x];
        int q = sparse[k][y - (1 << k) + 1];
        return depth[p] < depth[q] ? p : q;
    };

    // Hierarchical bitsets support predecessor/successor in O(log_64 N).
    vector<vector<unsigned long long>> bits;
    int words = (N + 63) / 64;
    while (true) {
        bits.push_back(vector<unsigned long long>(words, 0));
        if (words == 1) break;
        words = (words + 63) / 64;
    }

    auto previousMarkedIndex = [&](auto &&self, int level, int limit) -> int {
        if (limit <= 0 || level >= (int)bits.size()) return -1;

        int word = limit >> 6;
        int bit = limit & 63;

        unsigned long long mask = 0;
        if (word < (int)bits[level].size()) {
            if (bit != 0) mask = bits[level][word] & ((1ULL << bit) - 1);
            if (mask) {
                return (word << 6) + 63 - __builtin_clzll(mask);
            }
        }

        int previousWord = self(self, level + 1, word);
        if (previousWord < 0) return -1;
        unsigned long long v = bits[level][previousWord];
        return (previousWord << 6) + 63 - __builtin_clzll(v);
    };

    auto nextMarkedIndex = [&](auto &&self, int level, int after) -> int {
        if (level >= (int)bits.size()) return -1;

        int word = after >> 6;
        int bit = after & 63;

        if (word < (int)bits[level].size()) {
            unsigned long long mask = 0;
            if (bit != 63) mask = bits[level][word] & (~0ULL << (bit + 1));
            if (mask) return (word << 6) + __builtin_ctzll(mask);
        }

        int nextWord = self(self, level + 1, word);
        if (nextWord < 0) return -1;
        unsigned long long v = bits[level][nextWord];
        return (nextWord << 6) + __builtin_ctzll(v);
    };

    auto setMarked = [&](int pos, bool on) {
        int index = pos;
        bool wanted = on;

        for (int level = 0; level < (int)bits.size(); ++level) {
            int word = index >> 6;
            int bit = index & 63;

            bool wasNonEmpty = bits[level][word] != 0;
            if (wanted) bits[level][word] |= (1ULL << bit);
            else bits[level][word] &= ~(1ULL << bit);
            bool isNonEmpty = bits[level][word] != 0;

            if (wasNonEmpty == isNonEmpty) break;
            index = word;
            wanted = isNonEmpty;
        }
    };

    vector<Query> queries(Q);
    const int HILBERT_POWER = 18;
    for (int i = 0; i < Q; ++i) {
        cin >> queries[i].l >> queries[i].r;
        queries[i].id = i;
        queries[i].ord = hilbertOrder(queries[i].l, queries[i].r, HILBERT_POWER);
    }

    sort(queries.begin(), queries.end(), [](const Query &a, const Query &b) {
        return a.ord < b.ord;
    });

    int activeCount = 0;
    ll savedWeight = 0;
    vector<ll> answer(Q);

    auto nearestCost = [&](int turbine) -> ll {
        if (activeCount == 0) return 0;

        int p = leafPos[turbine];
        int best = -1;

        int prv = previousMarkedIndex(previousMarkedIndex, 0, p);
        if (prv != -1) best = lca(turbine, leafAt[prv]);

        int nxt = nextMarkedIndex(nextMarkedIndex, 0, p);
        if (nxt != -1) {
            int candidate = lca(turbine, leafAt[nxt]);
            if (best == -1 || value[candidate] < value[best]) best = candidate;
        }

        return value[best];
    };

    auto addTurbine = [&](int x) {
        if (activeCount > 0) savedWeight += nearestCost(x);
        setMarked(leafPos[x], true);
        ++activeCount;
    };

    auto removeTurbine = [&](int x) {
        setMarked(leafPos[x], false);
        --activeCount;
        if (activeCount > 0) savedWeight -= nearestCost(x);
    };

    int currentL = 0, currentR = -1;

    for (const auto &q : queries) {
        while (currentL > q.l) addTurbine(--currentL);
        while (currentR < q.r) addTurbine(++currentR);
        while (currentL < q.l) removeTurbine(currentL++);
        while (currentR > q.r) removeTurbine(currentR--);

        answer[q.id] = mstWeight - savedWeight;
    }

    for (ll x : answer) cout << x << '\n';
    return 0;
}
