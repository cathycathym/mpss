#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>

using namespace std;

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

        graph[edgeA[id]].push_back(make_pair(edgeB[id], id));
        graph[edgeB[id]].push_back(make_pair(edgeA[id], id));
    }

    // Find all bridges using an iterative DFS.
    vector<int> tin(n, 0), low(n, 0);
    vector<int> parentVertex(n, -1), parentEdge(n, -1);
    vector<size_t> nextEdge(n, 0);
    vector<char> isBridge(m, false);
    int timer = 0;

    for (int start = 0; start < n; ++start) {
        if (tin[start] != 0) continue;

        vector<int> stack;
        stack.push_back(start);
        tin[start] = low[start] = ++timer;

        while (!stack.empty()) {
            int v = stack.back();

            if (nextEdge[v] < graph[v].size()) {
                int to = graph[v][nextEdge[v]].first;
                int edgeId = graph[v][nextEdge[v]].second;
                ++nextEdge[v];

                if (edgeId == parentEdge[v]) continue;

                if (tin[to] == 0) {
                    parentVertex[to] = v;
                    parentEdge[to] = edgeId;
                    tin[to] = low[to] = ++timer;
                    stack.push_back(to);
                } else {
                    low[v] = min(low[v], tin[to]);
                }
            } else {
                stack.pop_back();

                if (parentEdge[v] != -1) {
                    int par = parentVertex[v];

                    if (low[v] > tin[par]) {
                        isBridge[parentEdge[v]] = true;
                    }

                    low[par] = min(low[par], low[v]);
                }
            }
        }
    }

    // Contract connected components after removing all bridges.
    vector<int> component(n, -1);
    int componentCount = 0;

    for (int start = 0; start < n; ++start) {
        if (component[start] != -1) continue;

        vector<int> stack;
        stack.push_back(start);
        component[start] = componentCount;

        while (!stack.empty()) {
            int v = stack.back();
            stack.pop_back();

            for (size_t i = 0; i < graph[v].size(); ++i) {
                int to = graph[v][i].first;
                int edgeId = graph[v][i].second;

                if (isBridge[edgeId] || component[to] != -1) continue;

                component[to] = componentCount;
                stack.push_back(to);
            }
        }

        ++componentCount;
    }

    // Bridges form a forest between the contracted components.
    vector<vector<pair<int, int>>> forest(componentCount);

    for (int id = 0; id < m; ++id) {
        if (!isBridge[id]) continue;

        int a = component[edgeA[id]];
        int b = component[edgeB[id]];
        forest[a].push_back(make_pair(b, id));
        forest[b].push_back(make_pair(a, id));
    }

    // Root each tree of the bridge forest.
    vector<int> parent(componentCount, -1);
    vector<int> parentBridge(componentCount, -1);
    vector<int> depth(componentCount, 0);
    vector<int> root(componentCount, -1);
    vector<int> order;
    order.reserve(componentCount);

    for (int start = 0; start < componentCount; ++start) {
        if (parent[start] != -1) continue;

        parent[start] = start;
        root[start] = start;

        vector<int> stack;
        stack.push_back(start);

        while (!stack.empty()) {
            int v = stack.back();
            stack.pop_back();
            order.push_back(v);

            for (size_t i = 0; i < forest[v].size(); ++i) {
                int to = forest[v][i].first;
                int edgeId = forest[v][i].second;

                if (parent[to] != -1) continue;

                parent[to] = v;
                parentBridge[to] = edgeId;
                depth[to] = depth[v] + 1;
                root[to] = start;
                stack.push_back(to);
            }
        }
    }

    int logSize = 1;
    while ((1LL << logSize) <= componentCount) ++logSize;

    vector<vector<int>>> up(logSize, vector<int>(componentCount));

    for (int v = 0; v < componentCount; ++v) {
        up[0][v] = parent[v];
    }

    for (int level = 1; level < logSize; ++level) {
        for (int v = 0; v < componentCount; ++v) {
            up[level][v] = up[level - 1][up[level - 1][v]];
        }
    }

    auto lca = [&](int a, int b) {
        if (root[a] != root[b]) return -1;

        if (depth[a] < depth[b]) swap(a, b);

        int difference = depth[a] - depth[b];
        for (int level = 0; level < logSize; ++level) {
            if (difference & (1LL << level)) {
                a = up[level][a];
            }
        }

        if (a == b) return a;

        for (int level = logSize - 1; level >= 0; --level) {
            if (up[level][a] != up[level][b]) {
                a = up[level][a];
                b = up[level][b];
            }
        }

        return parent[a];
    };

    vector<long long> needUp(componentCount, 0);
    vector<long long> needDown(componentCount, 0);

    int p;
    cin >> p;

    for (int i = 0; i < p; ++i) {
        int x, y;
        cin >> x >> y;
        --x;
        --y;

        int from = component[x];
        int to = component[y];

        if (from == to) continue;

        int common = lca(from, to);

        ++needUp[from];
        --needUp[common];

        ++needDown[to];
        --needDown[common];
    }

    string answer(m, 'B');

    // Accumulate path requirements from children to parents.
    for (int i = componentCount - 1; i >= 0; --i) {
        int v = order[i];

        if (parent[v] == v) continue;

        int par = parent[v];
        int edgeId = parentBridge[v];

        int requiredFrom = -1;
        int requiredTo = -1;

        if (needUp[v] > 0) {
            requiredFrom = v;
            requiredTo = par;
        } else if (needDown[v] > 0) {
            requiredFrom = par;
            requiredTo = v;
        }

        if (requiredFrom != -1) {
            int inputFrom = component[edgeA[edgeId]];
            int inputTo = component[edgeB[edgeId]];

            answer[edgeId] =
                (inputFrom == requiredFrom && inputTo == requiredTo) ? 'R' : 'L';
        }

        needUp[par] += needUp[v];
        needDown[par] += needDown[v];
    }

    cout << answer << '\n';
    return 0;
}