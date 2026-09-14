#include <bits/stdc++.h>
using namespace std;

using ll = long long;

struct Edge {
    int u, v;
    ll w;
    bool operator<(const Edge& other) const { return w < other.w; }
};

struct DSU {
    vector<int> p;
    DSU(int n) : p(n) { iota(p.begin(), p.end(), 0); }
    int find(int x) { return p[x] == x ? x : p[x] = find(p[x]); }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M, Q;
    cin >> N >> M >> Q;
    vector<Edge> edges(M);
    for (auto &e : edges) cin >> e.u >> e.v >> e.w;

    // Subtask 1: the graph itself is the numbered path.
    bool isPath = (M == N - 1);
    vector<ll> pathEdge(max(0, N - 1), -1);
    if (isPath) {
        for (auto &e : edges) {
            if (abs(e.u - e.v) != 1) {
                isPath = false;
                break;
            }
            pathEdge[min(e.u, e.v)] = e.w;
        }
        for (ll x : pathEdge) if (x < 0) isPath = false;
    }

    vector<pair<int,int>> queries(Q);
    for (auto &x : queries) cin >> x.first >> x.second;

    if (isPath) {
        vector<ll> pref(N, 0);
        for (int i = 0; i + 1 < N; ++i) pref[i + 1] = pref[i] + pathEdge[i];
        for (auto [l, r] : queries) {
            // Keep exactly the edges outside the free interval.
            cout << pref[l] + (pref[N - 1] - pref[r]) << '\n';
        }
        return 0;
    }

    /*
      Easy-to-hard ladder:

      1. Path graph:
         The interval splits the path into a left and right part.  Every edge
         outside [l,r] is necessary and every edge inside is unnecessary.
         O(1) per query after O(N) prefixes.

      2. Small total interval length:
         Build an MST.  For a set S of shore-connected turbines, the answer is

             MST_weight - MST_weight_of(S under bottleneck distances in the MST).

         The bottleneck distance between a,b is the maximum edge on their MST
         path.  A Kruskal reconstruction tree represents that distance as the
         weight of LCA(a,b).

      3. Intervals of two turbines:
         The second MST has one edge only, namely LCA(l,r), so O(log N).

      4. General fallback:
         Build the virtual tree of all terminals in [l,r].  Every virtual-tree
         branching node contributes its reconstruction-tree weight.  This is
         exact in O(k log k), k=r-l+1.

      The small-interval strategy cannot be retained for unrestricted intervals:
      a query may contain Theta(N) terminals, so explicitly constructing its
      virtual tree is too expensive.  A full-score solution requires an
      additional range-query structure over the reconstruction tree.
    */

    sort(edges.begin(), edges.end());

    int maxNodes = 2 * N;
    vector<vector<int>> child(maxNodes);
    vector<ll> nodeWeight(maxNodes, 0);
    vector<int> compRoot(maxNodes);
    iota(compRoot.begin(), compRoot.end(), 0);

    DSU dsu(N);
    int tot = N;
    ll mstWeight = 0;

    for (const Edge &e : edges) {
        int a = dsu.find(e.u), b = dsu.find(e.v);
        if (a == b) continue;

        int x = compRoot[a], y = compRoot[b];
        child[tot].push_back(x);
        child[tot].push_back(y);
        nodeWeight[tot] = e.w;
        mstWeight += e.w;

        dsu.p[b] = a;
        compRoot[a] = tot;
        ++tot;
    }

    int root = compRoot[dsu.find(0)];
    int LOG = 1;
    while ((1 << LOG) <= tot) ++LOG;

    vector<vector<int>> up(LOG, vector<int>(tot, root));
    vector<int> tin(tot), tout(tot), depth(tot);
    int timer = 0;

    vector<pair<int,int>> st;
    st.push_back({root, 0});
    up[0][root] = root;

    while (!st.empty()) {
        int v = st.back().first;
        int &state = st.back().second;
        if (state == 0) {
            tin[v] = timer++;
            if (v < N) {
                tout[v] = timer;
                st.pop_back();
                continue;
            }
        }
        if (state < (int)child[v].size()) {
            int to = child[v][state++];
            depth[to] = depth[v] + 1;
            up[0][to] = v;
            st.push_back({to, 0});
        } else {
            tout[v] = timer;
            st.pop_back();
        }
    }

    for (int j = 1; j < LOG; ++j)
        for (int v = 0; v < tot; ++v)
            up[j][v] = up[j - 1][up[j - 1][v]];

    auto ancestor = [&](int a, int b) {
        return tin[a] <= tin[b] && tout[b] <= tout[a];
    };

    auto lca = [&](int a, int b) {
        if (ancestor(a, b)) return a;
        if (ancestor(b, a)) return b;
        int x = a;
        for (int j = LOG - 1; j >= 0; --j)
            if (!ancestor(up[j][x], b)) x = up[j][x];
        return up[0][x];
    };

    // Subtask 6: intervals [0,r].  In an ultrametric, adding a terminal adds
    // its minimum bottleneck distance to an earlier terminal.
    vector<ll> prefixMST(N, 0);
    set<pair<int,int>> active;
    active.insert({tin[0], 0});
    for (int x = 1; x < N; ++x) {
        auto it = active.lower_bound({tin[x], x});
        ll best = (1LL << 62);
        if (it != active.end()) best = min(best, nodeWeight[lca(x, it->second)]);
        if (it != active.begin()) {
            --it;
            best = min(best, nodeWeight[lca(x, it->second)]);
        }
        prefixMST[x] = prefixMST[x - 1] + best;
        active.insert({tin[x], x});
    }

    map<pair<int,int>, ll> memo;

    for (auto [l, r] : queries) {
        auto key = make_pair(l, r);
        auto cached = memo.find(key);
        if (cached != memo.end()) {
            cout << cached->second << '\n';
            continue;
        }

        ll removed = 0;

        if (l == r) {
            removed = 0;
        } else if (l == 0) {
            removed = prefixMST[r];
        } else {
            vector<int> nodes;
            nodes.reserve(2 * (r - l + 1));

            for (int x = l; x <= r; ++x) nodes.push_back(x);
            sort(nodes.begin(), nodes.end(), [&](int a, int b) {
                return tin[a] < tin[b];
            });

            int original = (int)nodes.size();
            for (int i = 0; i + 1 < original; ++i)
                nodes.push_back(lca(nodes[i], nodes[i + 1]));

            sort(nodes.begin(), nodes.end(), [&](int a, int b) {
                return tin[a] < tin[b];
            });
            nodes.erase(unique(nodes.begin(), nodes.end()), nodes.end());

            vector<int> parent(nodes.size(), -1), degree(nodes.size(), 0), stackIdx;
            stackIdx.reserve(nodes.size());

            for (int i = 0; i < (int)nodes.size(); ++i) {
                while (!stackIdx.empty() &&
                       !ancestor(nodes[stackIdx.back()], nodes[i])) {
                    stackIdx.pop_back();
                }
                if (!stackIdx.empty()) {
                    parent[i] = stackIdx.back();
                    ++degree[parent[i]];
                }
                stackIdx.push_back(i);
            }

            for (int i = 0; i < (int)nodes.size(); ++i) {
                // A branching reconstruction-tree node merges two terminal groups.
                if (nodes[i] >= N && degree[i] >= 2)
                    removed += nodeWeight[nodes[i]];
            }
        }

        ll answer = mstWeight - removed;
        memo[key] = answer;
        cout << answer << '\n';
    }

    return 0;
}
