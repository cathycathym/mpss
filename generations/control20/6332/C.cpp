#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>
#include <cstdint>

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

    int insert(int oldRoot, int left, int right, int position) {
        int current = static_cast<int>(nodes.size());
        nodes.push_back(nodes[oldRoot]);

        ++nodes[current].count;
        nodes[current].sum += values[position];

        if (left != right) {
            int middle = (left + right) / 2;
            if (position <= middle) {
                int newLeft =
                    insert(nodes[oldRoot].left, left, middle, position);
                nodes[current].left = newLeft;
            } else {
                int newRight =
                    insert(nodes[oldRoot].right, middle + 1, right, position);
                nodes[current].right = newRight;
            }
        }

        return current;
    }

    int pathCount(int rootU, int rootV, int rootLca) const {
        return nodes[rootU].count + nodes[rootV].count
             - 2 * nodes[rootLca].count;
    }

    int64 maximumAffordableCount(
        int rootU, int rootV, int rootLca, int64 budget
    ) const {
        int left = 0;
        int right = static_cast<int>(values.size()) - 1;
        int64 answer = 0;

        int a = rootU;
        int b = rootV;
        int c = rootLca;

        while (left < right) {
            int aLeft = nodes[a].left;
            int bLeft = nodes[b].left;
            int cLeft = nodes[c].left;

            int64 leftSum =
                nodes[aLeft].sum + nodes[bLeft].sum
                - 2 * nodes[cLeft].sum;

            int leftCount =
                nodes[aLeft].count + nodes[bLeft].count
                - 2 * nodes[cLeft].count;

            int middle = (left + right) / 2;

            if (leftSum <= budget) {
                answer += leftCount;
                budget -= leftSum;

                a = nodes[a].right;
                b = nodes[b].right;
                c = nodes[c].right;
                left = middle + 1;
            } else {
                a = aLeft;
                b = bLeft;
                c = cLeft;
                right = middle;
            }
        }

        int leafCount =
            nodes[a].count + nodes[b].count - 2 * nodes[c].count;

        answer += min<int64>(leafCount, budget / values[left]);
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
        int edge;
        int64 cost;
        cin >> edge >> cost;
        --edge;
        edgeCosts[edge].push_back(cost);
        compressedCosts.push_back(cost);
    }

    sort(compressedCosts.begin(), compressedCosts.end());
    compressedCosts.erase(
        unique(compressedCosts.begin(), compressedCosts.end()),
        compressedCosts.end()
    );

    vector<int> parent(N, -1);
    vector<int> parentEdge(N, -1);
    vector<int> depth(N, 0);
    vector<int> order;
    order.reserve(N);

    parent[0] = 0;
    order.push_back(0);

    for (size_t index = 0; index < order.size(); ++index) {
        int vertex = order[index];

        for (const auto& connection : graph[vertex]) {
            int next = connection.first;
            int edge = connection.second;

            if (next == parent[vertex]) {
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
    for (int vertex = 0; vertex < N; ++vertex) {
        up[0][vertex] = parent[vertex];
    }

    for (int level = 1; level < LOG; ++level) {
        for (int vertex = 0; vertex < N; ++vertex) {
            up[level][vertex] = up[level - 1][up[level - 1][vertex]];
        }
    }

    auto lca = [&](int first, int second) {
        if (depth[first] < depth[second]) {
            swap(first, second);
        }

        int difference = depth[first] - depth[second];
        for (int level = 0; level < LOG; ++level) {
            if ((difference >> level) & 1) {
                first = up[level][first];
            }
        }

        if (first == second) {
            return first;
        }

        for (int level = LOG - 1; level >= 0; --level) {
            if (up[level][first] != up[level][second]) {
                first = up[level][first];
                second = up[level][second];
            }
        }

        return parent[first];
    };

    PersistentSegmentTree persistentTree(compressedCosts);
    persistentTree.nodes.reserve(static_cast<size_t>(M) * 20 + 1);

    vector<int> version(N, 0);

    for (size_t index = 1; index < order.size(); ++index) {
        int vertex = order[index];
        int root = version[parent[vertex]];

        for (int64 cost : edgeCosts[parentEdge[vertex]]) {
            int position = static_cast<int>(
                lower_bound(
                    compressedCosts.begin(), compressedCosts.end(), cost
                ) - compressedCosts.begin()
            );

            root = persistentTree.insert(
                root, 0, static_cast<int>(compressedCosts.size()) - 1, position
            );
        }

        version[vertex] = root;
    }

    for (int query = 0; query < Q; ++query) {
        int source, target;
        int64 gold, silver;
        cin >> source >> target >> gold >> silver;
        --source;
        --target;

        int ancestor = lca(source, target);

        int totalCheckpoints = persistentTree.pathCount(
            version[source], version[target], version[ancestor]
        );

        int64 silverPaid = persistentTree.maximumAffordableCount(
            version[source], version[target], version[ancestor], silver
        );

        int64 requiredGold =
            static_cast<int64>(totalCheckpoints) - silverPaid;

        if (requiredGold > gold) {
            cout << -1 << '\n';
        } else {
            cout << gold - requiredGold << '\n';
        }
    }

    return 0;
}
