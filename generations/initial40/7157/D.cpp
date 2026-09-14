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

    bool unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return false;
        if (size[a] < size[b]) swap(a, b);
        parent[b] = a;
        size[a] += size[b];
        return true;
    }
};

struct Edge {
    int u, v, capacity;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, W;
    cin >> N >> W;

    vector<vector<int> > C(N, vector<int>(N));
    vector<vector<int> > B(N, vector<int>(N));

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

    vector<pair<int, int> > eligible;
    for (int i = 0; i < N; ++i) {
        for (int j = i + 1; j < N; ++j) {
            if (C[i][j] + B[i][j] >= W) {
                eligible.push_back(make_pair(i, j));
            }
        }
    }

    const auto buildTree =
        [&](const vector<vector<int> >& target) -> vector<Edge> {
            vector<Edge> edges;
            edges.reserve(eligible.size());

            for (size_t k = 0; k < eligible.size(); ++k) {
                int u = eligible[k].first;
                int v = eligible[k].second;
                edges.push_back(Edge{u, v, target[u][v]});
            }

            sort(edges.begin(), edges.end(),
                 [](const Edge& x, const Edge& y) {
                     return x.capacity > y.capacity;
                 });

            DSU dsu(N);
            vector<Edge> tree;
            tree.reserve(N - 1);

            for (size_t i = 0; i < edges.size(); ++i) {
                if (dsu.unite(edges[i].u, edges[i].v)) {
                    tree.push_back(edges[i]);
                    if (static_cast<int>(tree.size()) == N - 1) break;
                }
            }

            if (static_cast<int>(tree.size()) != N - 1) {
                return vector<Edge>();
            }

            vector<vector<pair<int, int> > > graph(N);
            for (size_t i = 0; i < tree.size(); ++i) {
                const Edge& e = tree[i];
                graph[e.u].push_back(make_pair(e.v, e.capacity));
                graph[e.v].push_back(make_pair(e.u, e.capacity));
            }

            for (int source = 0; source < N; ++source) {
                vector<int> bottleneck(N, -1);
                vector<int> stack;
                stack.push_back(source);
                bottleneck[source] = W;

                while (!stack.empty()) {
                    int u = stack.back();
                    stack.pop_back();

                    for (size_t i = 0; i < graph[u].size(); ++i) {
                        int v = graph[u][i].first;
                        int capacity = graph[u][i].second;
                        if (bottleneck[v] == -1) {
                            bottleneck[v] = min(bottleneck[u], capacity);
                            stack.push_back(v);
                        }
                    }
                }

                for (int v = source + 1; v < N; ++v) {
                    if (bottleneck[v] != target[source][v]) {
                        return vector<Edge>();
                    }
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

    for (size_t i = 0; i < carTree.size(); ++i) {
        const Edge& e = carTree[i];
        cout << e.u << ' ' << e.v << ' ' << W - e.capacity << '\n';
    }

    for (size_t i = 0; i < bikeTree.size(); ++i) {
        const Edge& e = bikeTree[i];
        cout << e.u << ' ' << e.v << ' ' << e.capacity << '\n';
    }

    return 0;
}