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
        graph[u].push_back({v, id});
        graph[v].push_back({u, id});
    }

    // Iterative Tarjan algorithm for vertex-biconnected components.
    vector<int> tin(n, 0), low(n, 0);
    vector<int> parent(n, -1), parentEdge(n, -1);
    vector<int> nextEdge(n, 0);
    vector<int> dfsStack;
    vector<int> edgeStack;
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
                auto [to, id] = graph[v][nextEdge[v]++];

                if (id == parentEdge[v]) continue;

                if (tin[to] == 0) {
                    parent[to] = v;
                    parentEdge[to] = id;
                    edgeStack.push_back(id);

                    tin[to] = low[to] = ++timer;
                    dfsStack.push_back(to);
                } else if (tin[to] < tin[v]) {
                    // Back edge directed toward an ancestor.
                    edgeStack.push_back(id);
                    low[v] = min(low[v], tin[to]);
                }
            } else {
                dfsStack.pop_back();

                if (parent[v] != -1) {
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

                        blocks.push_back(move(vertices));
                    }
                }
            }
        }
    }

    int blockCount = static_cast<int>(blocks.size());
    int totalNodes = n + blockCount;

    // Build the block-cut forest.
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
        vector<int> stack;
        stack.push_back(root);
        treeParent[root] = -1;

        while (!stack.empty()) {
            int v = stack.back();
            stack.pop_back();
            order.push_back(v);

            for (int to : tree[v]) {
                if (to == treeParent[v]) continue;
                if (treeParent[to] != -2) continue;

                treeParent[to] = v;
                stack.push_back(to);
            }
        }

        int64 totalOriginal = 0;
        for (int v : order) {
            if (v < n) ++totalOriginal;
        }

        for (auto it = order.rbegin(); it != order.rend(); ++it) {
            int v = *it;
            subtree[v] += (v < n ? 1 : 0);

            if (treeParent[v] != -1) {
                subtree[treeParent[v]] += subtree[v];
            }
        }

        for (int v : order) {
            componentSize[v] = totalOriginal;
        }
    }

    // Size of the side containing vertex v after removing incidence B-v.
    auto sideSize = [&](int blockNode, int v) -> int64 {
        if (treeParent[v] == blockNode) {
            return subtree[v];
        }
        // Then v is the parent of blockNode.
        return componentSize[blockNode] - subtree[blockNode];
    };

    vector<int64> blockSquareSum(blockCount, 0);

    for (int b = 0; b < blockCount; ++b) {
        int blockNode = n + b;

        for (int v : blocks[b]) {
            int64 x = sideSize(blockNode, v);
            blockSquareSum[b] += x * x;
        }
    }

    vector<int64> invalidSquareSum(n, 0);

    for (int b = 0; b < blockCount; ++b) {
        int blockNode = n + b;

        for (int c : blocks[b]) {
            int64 ownSide = sideSize(blockNode, c);
            invalidSquareSum[c] +=
                blockSquareSum[b] - ownSide * ownSide;
        }
    }

    int64 answer = 0;

    for (int c = 0; c < n; ++c) {
        int64 otherVertices = componentSize[c] - 1;
        answer += otherVertices * otherVertices - invalidSquareSum[c];
    }

    cout << answer << '\n';
    return 0;
}