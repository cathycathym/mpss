#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

struct Frame {
    int v;
    int parentEdge;
    int nextEdge;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    vector<int> eu(m), ev(m);
    vector<vector<int>> graph(n);

    for (int i = 0; i < m; ++i) {
        cin >> eu[i] >> ev[i];
        --eu[i];
        --ev[i];
        graph[eu[i]].push_back(i);
        graph[ev[i]].push_back(i);
    }

    // Iterative Tarjan algorithm for vertex-biconnected components.
    vector<int> tin(n, 0), low(n, 0);
    vector<int> edgeStack;
    vector<vector<int>> blocks;
    vector<int> seenInBlock(n, 0);
    int timer = 0;

    for (int start = 0; start < n; ++start) {
        if (tin[start] != 0) continue;

        vector<Frame> dfs;
        tin[start] = low[start] = ++timer;
        dfs.push_back({start, -1, 0});

        while (!dfs.empty()) {
            Frame &fr = dfs.back();
            int v = fr.v;

            if (fr.nextEdge < (int)graph[v].size()) {
                int id = graph[v][fr.nextEdge++];
                if (id == fr.parentEdge) continue;

                int to = eu[id] ^ ev[id] ^ v;

                if (tin[to] == 0) {
                    edgeStack.push_back(id);
                    tin[to] = low[to] = ++timer;
                    dfs.push_back({to, id, 0});
                } else if (tin[to] < tin[v]) {
                    // Back edge to an ancestor.
                    edgeStack.push_back(id);
                    low[v] = min(low[v], tin[to]);
                }
            } else {
                int parentEdge = fr.parentEdge;
                dfs.pop_back();

                if (parentEdge != -1) {
                    int parent = eu[parentEdge] ^ ev[parentEdge] ^ v;
                    low[parent] = min(low[parent], low[v]);

                    if (low[v] >= tin[parent]) {
                        int token = (int)blocks.size() + 1;
                        vector<int> vertices;

                        while (true) {
                            int id = edgeStack.back();
                            edgeStack.pop_back();

                            int a = eu[id];
                            int b = ev[id];

                            if (seenInBlock[a] != token) {
                                seenInBlock[a] = token;
                                vertices.push_back(a);
                            }
                            if (seenInBlock[b] != token) {
                                seenInBlock[b] = token;
                                vertices.push_back(b);
                            }

                            if (id == parentEdge) break;
                        }

                        blocks.push_back(std::move(vertices));
                    }
                }
            }
        }
    }

    int blockCount = (int)blocks.size();
    int totalNodes = n + blockCount;

    // Build the block-cut forest.
    vector<vector<int>> tree(totalNodes);
    for (int i = 0; i < blockCount; ++i) {
        int blockNode = n + i;
        for (int v : blocks[i]) {
            tree[blockNode].push_back(v);
            tree[v].push_back(blockNode);
        }
    }

    vector<int> parent(totalNodes, -2);
    vector<long long> subtree(totalNodes, 0);
    vector<long long> componentSize(totalNodes, 0);
    vector<int> order;

    // Root each tree, calculate subtree weights and component sizes.
    for (int root = 0; root < totalNodes; ++root) {
        if (parent[root] != -2) continue;

        order.clear();
        vector<int> stack;
        stack.push_back(root);
        parent[root] = -1;

        long long totalWeight = 0;

        while (!stack.empty()) {
            int v = stack.back();
            stack.pop_back();
            order.push_back(v);

            if (v < n) ++totalWeight;

            for (int to : tree[v]) {
                if (to == parent[v]) continue;
                if (parent[to] != -2) continue;
                parent[to] = v;
                stack.push_back(to);
            }
        }

        for (int v : order) {
            subtree[v] = (v < n ? 1LL : 0LL);
            componentSize[v] = totalWeight;
        }

        for (int i = (int)order.size() - 1; i >= 0; --i) {
            int v = order[i];
            if (parent[v] != -1) {
                subtree[parent[v]] += subtree[v];
            }
        }
    }

    long long answer = 0;

    // Triples whose median is the change intersection itself.
    for (int c = 0; c < n; ++c) {
        long long K = componentSize[c];
        long long sumSquares = 0;

        for (int to : tree[c]) {
            long long branchSize;
            if (parent[to] == c) {
                branchSize = subtree[to];
            } else {
                branchSize = K - subtree[c];
            }
            sumSquares += branchSize * branchSize;
        }

        answer += (K - 1) * (K - 1) - sumSquares;
    }

    // Triples whose median is a biconnected-component node.
    for (int i = 0; i < blockCount; ++i) {
        int blockNode = n + i;
        long long K = componentSize[blockNode];
        long long sumSquares = 0;

        vector<long long> branchSizes;
        branchSizes.reserve(tree[blockNode].size());

        for (int v : tree[blockNode]) {
            long long branchSize;
            if (parent[v] == blockNode) {
                branchSize = subtree[v];
            } else {
                branchSize = K - subtree[blockNode];
            }

            branchSizes.push_back(branchSize);
            sumSquares += branchSize * branchSize;
        }

        for (long long w : branchSizes) {
            answer += (K - w) * (K - w) - (sumSquares - w * w);
        }
    }

    cout << answer << '\n';
    return 0;
}