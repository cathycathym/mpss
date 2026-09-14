#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <utility>

using namespace std;

class DSU {
    vector<int> parent, size;
public:
    explicit DSU(int n) : parent(n), size(n, 1) {
        for (int i = 0; i < n; ++i) parent[i] = i;
    }

    int find(int x) {
        while (parent[x] != x) {
            parent[x] = parent[parent[x]];
            x = parent[x];
        }
        return x;
    }

    void unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return;
        if (size[a] < size[b]) swap(a, b);
        parent[b] = a;
        size[a] += size[b];
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    vector<int> edgeA(m), edgeB(m);
    vector<vector<pair<int, int>>> graph(n);

    for (int id = 0; id < m; ++id) {
        cin >> edgeA[id] >> edgeB[id];
        --edgeA[id];
        --edgeB[id];
        graph[edgeA[id]].push_back({edgeB[id], id});
        graph[edgeB[id]].push_back({edgeA[id], id});
    }

    // Iterative bridge-finding DFS.
    vector<int> tin(n, 0), low(n, 0);
    vector<int> parentVertex(n, -1), parentEdge(n, -1), nextEdge(n, 0);
    vector<char> isBridge(m, false);
    int timer = 0;

    for (int start = 0; start < n; ++start) {
        if (tin[start] != 0) continue;

        vector<int> stack;
        tin[start] = low[start] = ++timer;
        stack.push_back(start);

        while (!stack.empty()) {
            int v = stack.back();

            if (nextEdge[v] < static_cast<int>(graph[v].size())) {
                auto [to, id] = graph[v][nextEdge[v]++];

                if (id == parentEdge[v]) continue;

                if (tin[to] == 0) {
                    parentVertex[to] = v;
                    parentEdge[to] = id;
                    tin[to] = low[to] = ++timer;
                    stack.push_back(to);
                } else {
                    low[v] = min(low[v], tin[to]);
                }
            } else {
                stack.pop_back();

                if (parentVertex[v] != -1) {
                    int par = parentVertex[v];
                    if (low[v] > tin[par]) {
                        isBridge[parentEdge[v]] = true;
                    }
                    low[par] = min(low[par], low[v]);
                }
            }
        }
    }

    // Contract all non-bridge edges.
    DSU dsu(n);
    for (int id = 0; id < m; ++id) {
        if (!isBridge[id]) {
            dsu.unite(edgeA[id], edgeB[id]);
        }
    }

    vector<int> rootToComponent(n, -1);
    vector<int> componentOf(n);
    int componentCount = 0;

    for (int v = 0; v < n; ++v) {
        int root = dsu.find(v);
        if (rootToComponent[root] == -1) {
            rootToComponent[root] = componentCount++;
        }
        componentOf[v] = rootToComponent[root];
    }

    // The bridges form a forest between contracted components.
    vector<vector<pair<int, int>>> tree(componentCount);
    for (int id = 0; id < m; ++id) {
        if (!isBridge[id]) continue;

        int a = componentOf[edgeA[id]];
        int b = componentOf[edgeB[id]];
        tree[a].push_back({b, id});
        tree[b].push_back({a, id});
    }

    vector<int> parent(componentCount, -2);
    vector<int> parentBridge(componentCount, -1);
    vector<int> depth(componentCount, 0);
    vector<int> treeRoot(componentCount, -1);
    vector<int> order;
    order.reserve(componentCount);

    for (int start = 0; start < componentCount; ++start) {
        if (parent[start] != -2) continue;

        parent[start] = -1;
        treeRoot[start] = start;

        vector<int> stack = {start};
        while (!stack.empty()) {
            int v = stack.back();
            stack.pop_back();
            order.push_back(v);

            for (auto [to, edgeId] : tree[v]) {
                if (edgeId == parentBridge[v] || parent[to] != -2) continue;

                parent[to] = v;
                parentBridge[to] = edgeId;
                depth[to] = depth[v] + 1;
                treeRoot[to] = start;
                stack.push_back(to);
            }
        }
    }

    int log = 1;
    while ((1 << log) <= componentCount) ++log;

    vector<vector<int>> ancestor(log, vector<int>(componentCount));
    for (int v = 0; v < componentCount; ++v) {
        ancestor[0][v] = (parent[v] == -1 ? v : parent[v]);
    }

    for (int k = 1; k < log; ++k) {
        for (int v = 0; v < componentCount; ++v) {
            ancestor[k][v] = ancestor[k - 1][ancestor[k - 1][v]];
        }
    }

    auto lca = [&](int a, int b) {
        if (treeRoot[a] != treeRoot[b]) return -1;

        if (depth[a] < depth[b]) swap(a, b);

        int difference = depth[a] - depth[b];
        for (int k = 0; k < log; ++k) {
            if (difference & (1 << k)) {
                a = ancestor[k][a];
            }
        }

        if (a == b) return a;

        for (int k = log - 1; k >= 0; --k) {
            if (ancestor[k][a] != ancestor[k][b]) {
                a = ancestor[k][a];
                b = ancestor[k][b];
            }
        }

        return parent[a];
    };

    vector<int> requireUp(componentCount, 0);
    vector<int> requireDown(componentCount, 0);

    int p;
    cin >> p;

    for (int i = 0; i < p; ++i) {
        int x, y;
        cin >> x >> y;
        --x;
        --y;

        int from = componentOf[x];
        int to = componentOf[y];

        if (from == to) continue;

        int common = lca(from, to);

        // Guaranteed existence implies common != -1.
        ++requireUp[from];
        --requireUp[common];

        ++requireDown[to];
        --requireDown[common];
    }

    string answer(m, 'B');

    // Aggregate requirements from children toward roots.
    for (int i = componentCount - 1; i >= 0; --i) {
        int v = order[i];
        int par = parent[v];

        if (par == -1) continue;

        int edgeId = parentBridge[v];
        int requiredFrom = -1;
        int requiredTo = -1;

        if (requireUp[v] > 0) {
            // Child component -> parent component.
            requiredFrom = v;
            requiredTo = par;
        } else if (requireDown[v] > 0) {
            // Parent component -> child component.
            requiredFrom = par;
            requiredTo = v;
        }

        if (requiredFrom != -1) {
            int firstComponent = componentOf[edgeA[edgeId]];
            int secondComponent = componentOf[edgeB[edgeId]];

            if (firstComponent == requiredFrom &&
                secondComponent == requiredTo) {
                answer[edgeId] = 'R';
            } else {
                answer[edgeId] = 'L';
            }
        }

        requireUp[par] += requireUp[v];
        requireDown[par] += requireDown[v];
    }

    cout << answer << '\n';
    return 0;
}