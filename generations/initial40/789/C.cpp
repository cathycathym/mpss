#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <string>

using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    vector<int> edgeA(m), edgeB(m);
    vector<vector<pair<int, int>>> graph(n);

    for (int i = 0; i < m; ++i) {
        cin >> edgeA[i] >> edgeB[i];
        --edgeA[i];
        --edgeB[i];
        graph[edgeA[i]].push_back({edgeB[i], i});
        graph[edgeB[i]].push_back({edgeA[i], i});
    }

    // Iterative Tarjan bridge search.
    vector<int> tin(n, 0), low(n, 0);
    vector<int> parentVertex(n, -1), parentEdge(n, -1);
    vector<size_t> nextEdge(n, 0);
    vector<char> isBridge(m, false);
    int timer = 0;

    for (int start = 0; start < n; ++start) {
        if (tin[start] != 0) {
            continue;
        }

        vector<int> stack;
        stack.push_back(start);
        tin[start] = low[start] = ++timer;

        while (!stack.empty()) {
            int v = stack.back();

            if (nextEdge[v] < graph[v].size()) {
                auto current = graph[v][nextEdge[v]++];
                int to = current.first;
                int edgeId = current.second;

                if (edgeId == parentEdge[v]) {
                    continue;
                }

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
                    int parent = parentVertex[v];
                    low[parent] = min(low[parent], low[v]);

                    if (low[v] > tin[parent]) {
                        isBridge[parentEdge[v]] = true;
                    }
                }
            }
        }
    }

    // Contract every connected component after removing bridges.
    vector<int> component(n, -1);
    int componentCount = 0;

    for (int start = 0; start < n; ++start) {
        if (component[start] != -1) {
            continue;
        }

        vector<int> stack;
        stack.push_back(start);
        component[start] = componentCount;

        while (!stack.empty()) {
            int v = stack.back();
            stack.pop_back();

            for (auto entry : graph[v]) {
                int to = entry.first;
                int edgeId = entry.second;

                if (isBridge[edgeId] || component[to] != -1) {
                    continue;
                }

                component[to] = componentCount;
                stack.push_back(to);
            }
        }

        ++componentCount;
    }

    // Construct the bridge forest.
    vector<vector<pair<int, int>>> forest(componentCount);

    for (int edgeId = 0; edgeId < m; ++edgeId) {
        if (!isBridge[edgeId]) {
            continue;
        }

        int x = component[edgeA[edgeId]];
        int y = component[edgeB[edgeId]];
        forest[x].push_back({y, edgeId});
        forest[y].push_back({x, edgeId});
    }

    // Root every tree in the forest.
    vector<int> parent(componentCount, -1);
    vector<int> depth(componentCount, 0);
    vector<int> root(componentCount, -1);
    vector<int> parentBridge(componentCount, -1);
    vector<int> order;
    order.reserve(componentCount);

    for (int start = 0; start < componentCount; ++start) {
        if (parent[start] != -1) {
            continue;
        }

        parent[start] = start;
        root[start] = start;

        vector<int> stack;
        stack.push_back(start);

        while (!stack.empty()) {
            int v = stack.back();
            stack.pop_back();
            order.push_back(v);

            for (auto entry : forest[v]) {
                int to = entry.first;
                int edgeId = entry.second;

                if (parent[to] != -1) {
                    continue;
                }

                parent[to] = v;
                parentBridge[to] = edgeId;
                depth[to] = depth[v] + 1;
                root[to] = start;
                stack.push_back(to);
            }
        }
    }

    int logSize = 1;
    while ((1LL << logSize) <= componentCount) {
        ++logSize;
    }

    vector<vector<int>> up(logSize, vector<int>(componentCount));
    for (int v = 0; v < componentCount; ++v) {
        up[0][v] = parent[v];
    }

    for (int level = 1; level < logSize; ++level) {
        for (int v = 0; v < componentCount; ++v) {
            up[level][v] = up[level - 1][up[level - 1][v]];
        }
    }

    auto lca = [&](int first, int second) {
        if (root[first] != root[second]) {
            return -1;
        }

        int a = first;
        int b = second;

        if (depth[a] < depth[b]) {
            swap(a, b);
        }

        int difference = depth[a] - depth[b];
        for (int level = 0; level < logSize; ++level) {
            if (difference & (1 << level)) {
                a = up[level][a];
            }
        }

        if (a == b) {
            return a;
        }

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

        if (from == to) {
            continue;
        }

        int ancestor = lca(from, to);

        // ancestor == -1 would mean the requirement is impossible.
        // The problem guarantees that a solution exists.
        if (ancestor == -1) {
            continue;
        }

        ++needUp[from];
        --needUp[ancestor];

        ++needDown[to];
        --needDown[ancestor];
    }

    string answer(m, 'B');

    // Accumulate path differences from children toward their parents.
    for (int index = componentCount - 1; index >= 0; --index) {
        int v = order[index];

        if (parent[v] == v) {
            continue;
        }

        int par = parent[v];
        int edgeId = parentBridge[v];

        int wantedFrom = -1;
        int wantedTo = -1;

        if (needUp[v] > 0) {
            wantedFrom = v;
            wantedTo = par;
        } else if (needDown[v] > 0) {
            wantedFrom = par;
            wantedTo = v;
        }

        if (wantedFrom != -1) {
            int inputFromComponent = component[edgeA[edgeId]];
            int inputToComponent = component[edgeB[edgeId]];

            if (inputFromComponent == wantedFrom &&
                inputToComponent == wantedTo) {
                answer[edgeId] = 'R';
            } else {
                answer[edgeId] = 'L';
            }
        }

        needUp[par] += needUp[v];
        needDown[par] += needDown[v];
    }

    cout << answer << '\n';
    return 0;
}