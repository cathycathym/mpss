#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
using namespace std;

using int64 = long long;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    vector<int> edgeU(m), edgeV(m);
    vector<vector<pair<int, int>>> graph(n);

    for (int id = 0; id < m; ++id) {
        int u, v;
        cin >> u >> v;
        --u;
        --v;

        edgeU[id] = u;
        edgeV[id] = v;
        graph[u].push_back(make_pair(v, id));
        graph[v].push_back(make_pair(u, id));
    }

    // Find all vertex-biconnected components with iterative Tarjan DFS.
    vector<int> tin(n, 0), low(n, 0);
    vector<int> parent(n, -1), parentEdge(n, -1);
    vector<int> nextEdge(n, 0), dfsStack, edgeStack;
    vector<vector<int>> blocks;
    vector<int> vertexMark(n, -1);

    int timer = 0;

    for (int root = 0; root < n; ++root) {
        if (tin[root] != 0) continue;

        tin[root] = low[root] = ++timer;
        dfsStack.push_back(root);

        while (!dfsStack.empty()) {
            int v = dfsStack.back();

            if (nextEdge[v] < static_cast<int>(graph[v].size())) {
                int to = graph[v][nextEdge[v]].first;
                int id = graph[v][nextEdge[v]].second;
                ++nextEdge[v];

                if (id == parentEdge[v]) continue;

                if (tin[to] == 0) {
                    parent[to] = v;
                    parentEdge[to] = id;
                    tin[to] = low[to] = ++timer;

                    edgeStack.push_back(id);
                    dfsStack.push_back(to);
                } else if (tin[to] < tin[v]) {
                    low[v] = min(low[v], tin[to]);
                    edgeStack.push_back(id);
                }
            } else {
                dfsStack.pop_back();

                if (parent[v] == -1) continue;

                int p = parent[v];
                low[p] = min(low[p], low[v]);

                if (low[v] >= tin[p]) {
                    int blockId = static_cast<int>(blocks.size());
                    vector<int> vertices;

                    while (true) {
                        int id = edgeStack.back();
                        edgeStack.pop_back();

                        int a = edgeU[id];
                        int b = edgeV[id];

                        if (vertexMark[a] != blockId) {
                            vertexMark[a] = blockId;
                            vertices.push_back(a);
                        }
                        if (vertexMark[b] != blockId) {
                            vertexMark[b] = blockId;
                            vertices.push_back(b);
                        }

                        if (id == parentEdge[v]) break;
                    }

                    blocks.push_back(vertices);
                }
            }
        }
    }

    // Construct the block-cut forest.
    int blockCount = static_cast<int>(blocks.size());
    int totalNodes = n + blockCount;
    vector<vector<int>> tree(totalNodes);

    for (int b = 0; b < blockCount; ++b) {
        int blockNode = n + b;

        for (int v : blocks[b]) {
            tree[blockNode].push_back(v);
            tree[v].push_back(blockNode);
        }
    }

    // Original vertices have weight 1; block nodes have weight 0.
    vector<int> treeParent(totalNodes, -2);
    vector<int64> subtree(totalNodes, 0);
    vector<int64> componentSize(totalNodes, 0);

    for (int root = 0; root < totalNodes; ++root) {
        if (treeParent[root] != -2) continue;

        vector<int> order;
        vector<int> stack(1, root);
        treeParent[root] = -1;

        while (!stack.empty()) {
            int v = stack.back();
            stack.pop_back();
            order.push_back(v);

            for (int to : tree[v]) {
                if (treeParent[to] != -2) continue;
                treeParent[to] = v;
                stack.push_back(to);
            }
        }

        int64 totalOriginal = 0;
        for (int v : order) {
            if (v < n) ++totalOriginal;
            subtree[v] = (v < n ? 1LL : 0LL);
            componentSize[v] = totalOriginal;
        }

        for (int i = static_cast<int>(order.size()) - 1; i >= 0; --i) {
            int v = order[i];
            if (treeParent[v] != -1) {
                subtree[treeParent[v]] += subtree[v];
            }
        }

        for (int v : order) {
            componentSize[v] = totalOriginal;
        }
    }

    // Number of original vertices on v's side of the incidence (blockNode, v).
    const auto sideSize = [&](int blockNode, int v) -> int64 {
        if (treeParent[v] == blockNode) {
            return subtree[v];
        }
        return componentSize[blockNode] - subtree[blockNode];
    };

    vector<int64> blockSquareSum(blockCount, 0);

    for (int b = 0; b < blockCount; ++b) {
        int blockNode = n + b;

        for (int v : blocks[b]) {
            int64 size = sideSize(blockNode, v);
            blockSquareSum[b] += size * size;
        }
    }

    vector<int64> invalidPairs(n, 0);

    for (int b = 0; b < blockCount; ++b) {
        int blockNode = n + b;

        for (int c : blocks[b]) {
            int64 cSide = sideSize(blockNode, c);
            invalidPairs[c] += blockSquareSum[b] - cSide * cSide;
        }
    }

    int64 answer = 0;

    for (int c = 0; c < n; ++c) {
        int64 choices = componentSize[c] - 1;
        answer += choices * choices - invalidPairs[c];
    }

    cout << answer << '\n';
    return 0;
}