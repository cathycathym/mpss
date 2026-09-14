#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

struct PersistentSegmentTree {
    struct Node {
        int left = 0;
        int right = 0;
        int count = 0;
        int64 sum = 0;
    };

    vector<Node> nodes;
    const vector<int64>& values;

    explicit PersistentSegmentTree(const vector<int64>& compressedValues)
        : values(compressedValues) {
        nodes.push_back(Node());
    }

    int insert(int oldRoot, int low, int high, int position) {
        int root = static_cast<int>(nodes.size());
        nodes.push_back(nodes[oldRoot]);
        ++nodes[root].count;
        nodes[root].sum += values[position];

        if (low != high) {
            int middle = (low + high) / 2;
            if (position <= middle) {
                nodes[root].left =
                    insert(nodes[oldRoot].left, low, middle, position);
            } else {
                nodes[root].right =
                    insert(nodes[oldRoot].right, middle + 1, high, position);
            }
        }

        return root;
    }

    int pathCount(int rootU, int rootV, int rootLca) const {
        return nodes[rootU].count + nodes[rootV].count
             - 2 * nodes[rootLca].count;
    }

    int64 maximumAffordableCount(
        int rootU,
        int rootV,
        int rootLca,
        int64 budget
    ) const {
        int low = 0;
        int high = static_cast<int>(values.size()) - 1;
        int a = rootU;
        int b = rootV;
        int c = rootLca;
        int64 answer = 0;

        while (low < high) {
            int middle = (low + high) / 2;

            int aLeft = nodes[a].left;
            int bLeft = nodes[b].left;
            int cLeft = nodes[c].left;

            int64 leftSum =
                nodes[aLeft].sum + nodes[bLeft].sum
                - 2 * nodes[cLeft].sum;

            int leftCount =
                nodes[aLeft].count + nodes[bLeft].count
                - 2 * nodes[cLeft].count;

            if (leftSum <= budget) {
                answer += leftCount;
                budget -= leftSum;

                a = nodes[a].right;
                b = nodes[b].right;
                c = nodes[c].right;
                low = middle + 1;
            } else {
                a = aLeft;
                b = bLeft;
                c = cLeft;
                high = middle;
            }
        }

        int leafCount =
            nodes[a].count + nodes[b].count - 2 * nodes[c].count;

        answer += min<int64>(leafCount, budget / values[low]);
        return answer;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M, Q;
    cin >> N >> M >> Q;

    vector<vector<pair<int, int>>> graph(N);
    for (int edge = 0; edge < N - 1; ++edge) {
        int a, b;
        cin >> a >> b;
        --a;
        --b;
        graph[a].push_back({b, edge});
        graph[b].push_back({a, edge});
    }

    vector<vector<int64>> edgeCosts(N - 1);
    vector<int64> compressedCosts;
    compressedCosts.reserve(M);

    for (int i = 0; i < M; ++i) {
        int road;
        int64 cost;
        cin >> road >> cost;
        --road;
        edgeCosts[road].push_back(cost);
        compressedCosts.push_back(cost);
    }

    sort(compressedCosts.begin(), compressedCosts.end());
    compressedCosts.erase(
        unique(compressedCosts.begin(), compressedCosts.end()),
        compressedCosts.end()
    );

    vector<int> parent(N, 0);
    vector<int> parentEdge(N, -1);
    vector<int> depth(N, 0);
    vector<int> order;
    order.reserve(N);
    order.push_back(0);

    for (size_t i = 0; i < order.size(); ++i) {
        int vertex = order[i];

        for (auto [next, edge] : graph[vertex]) {
            if (next == parent[vertex] && vertex != 0) {
                continue;
            }
            if (next == 0 || (next != 0 && parentEdge[next] != -1)) {
                continue;
            }

            parent[next] = vertex;
            parentEdge[next] = edge;
            depth[next] = depth[vertex] + 1;
            order.push_back(next);
        }
    }

    int LOG = 1;
    while ((1 << LOG) <= N) {
        ++LOG;
    }

    vector<vector<int>> up(LOG, vector<int>(N));
    up[0] = parent;

    for (int level = 1; level < LOG; ++level) {
        for (int vertex = 0; vertex < N; ++vertex) {
            up[level][vertex] =
                up[level - 1][up[level - 1][vertex]];
        }
    }

    auto lca = [&](int u, int v) {
        if (depth[u] < depth[v]) {
            swap(u, v);
        }

        int difference = depth[u] - depth[v];
        for (int level = 0; level < LOG; ++level) {
            if ((difference >> level) & 1) {
                u = up[level][u];
            }
        }

        if (u == v) {
            return u;
        }

        for (int level = LOG - 1; level >= 0; --level) {
            if (up[level][u] != up[level][v]) {
                u = up[level][u];
                v = up[level][v];
            }
        }

        return parent[u];
    };

    PersistentSegmentTree tree(compressedCosts);
    tree.nodes.reserve(static_cast<size_t>(M) * 20 + 1);

    vector<int> version(N, 0);

    for (size_t i = 1; i < order.size(); ++i) {
        int vertex = order[i];
        int root = version[parent[vertex]];

        for (int64 cost : edgeCosts[parentEdge[vertex]]) {
            int position = static_cast<int>(
                lower_bound(
                    compressedCosts.begin(),
                    compressedCosts.end(),
                    cost
                ) - compressedCosts.begin()
            );

            root = tree.insert(
                root,
                0,
                static_cast<int>(compressedCosts.size()) - 1,
                position
            );
        }

        version[vertex] = root;
    }

    while (Q--) {
        int source, target;
        int64 gold, silver;
        cin >> source >> target >> gold >> silver;
        --source;
        --target;

        int ancestor = lca(source, target);

        int totalCheckpoints = tree.pathCount(
            version[source],
            version[target],
            version[ancestor]
        );

        int64 paidWithSilver = tree.maximumAffordableCount(
            version[source],
            version[target],
            version[ancestor],
            silver
        );

        int64 requiredGold = totalCheckpoints - paidWithSilver;

        if (requiredGold > gold) {
            cout << -1 << '\n';
        } else {
            cout << gold - requiredGold << '\n';
        }
    }

    return 0;
}
