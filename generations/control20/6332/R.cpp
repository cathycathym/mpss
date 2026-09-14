#include <bits/stdc++.h>
using namespace std;

struct Node {
    long long sum;
    int left, right, count;
    Node(long long s = 0, int l = 0, int r = 0, int c = 0)
        : sum(s), left(l), right(r), count(c) {}
};

class PersistentSegmentTree {
    vector<Node> tree;
    const vector<long long>& values;

public:
    explicit PersistentSegmentTree(const vector<long long>& compressedValues, int updates)
        : values(compressedValues) {
        int height = 0;
        while ((1LL << height) < (int)values.size()) ++height;
        tree.reserve(1LL + 1LL * updates * (height + 1));
        tree.emplace_back();
    }

    int update(int previous, int low, int high, int position, long long value) {
        int current = (int)tree.size();
        tree.push_back(tree[previous]);
        ++tree[current].count;
        tree[current].sum += value;

        if (low != high) {
            int middle = (low + high) / 2;
            if (position <= middle) {
                tree[current].left =
                    update(tree[previous].left, low, middle, position, value);
            } else {
                tree[current].right =
                    update(tree[previous].right, middle + 1, high, position, value);
            }
        }
        return current;
    }

    int pathCount(int first, int second, int lca) const {
        return tree[first].count + tree[second].count - 2 * tree[lca].count;
    }

    long long maximumAffordable(
        int first, int second, int lca, int low, int high, long long& budget
    ) const {
        long long count =
            (long long)tree[first].count + tree[second].count - 2LL * tree[lca].count;

        if (count == 0) return 0;

        long long sum =
            tree[first].sum + tree[second].sum - 2LL * tree[lca].sum;

        if (sum <= budget) {
            budget -= sum;
            return count;
        }

        if (low == high) {
            long long bought = min(count, budget / values[low]);
            budget -= bought * values[low];
            return bought;
        }

        int firstLeft = tree[first].left;
        int secondLeft = tree[second].left;
        int lcaLeft = tree[lca].left;

        long long leftCount =
            (long long)tree[firstLeft].count + tree[secondLeft].count
            - 2LL * tree[lcaLeft].count;
        long long leftSum =
            tree[firstLeft].sum + tree[secondLeft].sum
            - 2LL * tree[lcaLeft].sum;

        int middle = (low + high) / 2;

        if (leftSum > budget) {
            return maximumAffordable(
                firstLeft, secondLeft, lcaLeft, low, middle, budget
            );
        }

        budget -= leftSum;
        return leftCount + maximumAffordable(
            tree[first].right,
            tree[second].right,
            tree[lca].right,
            middle + 1,
            high,
            budget
        );
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

    vector<vector<long long>> edgeCosts(N - 1);
    vector<long long> compressedCosts;
    compressedCosts.reserve(M);

    for (int i = 0; i < M; ++i) {
        int edge;
        long long cost;
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

    const int LOG = 18;
    vector<array<int, LOG>> ancestor(N);
    vector<int> parent(N, -1), parentEdge(N, -1), depth(N);
    vector<int> order;
    order.reserve(N);

    parent[0] = 0;
    for (int level = 0; level < LOG; ++level) ancestor[0][level] = 0;

    vector<int> stack;
    stack.push_back(0);
    order.push_back(0);

    while (!stack.empty()) {
        int city = stack.back();
        stack.pop_back();

        for (const auto& connection : graph[city]) {
            int next = connection.first;
            int edge = connection.second;
            if (parent[next] != -1) continue;

            parent[next] = city;
            parentEdge[next] = edge;
            depth[next] = depth[city] + 1;
            ancestor[next][0] = city;

            for (int level = 1; level < LOG; ++level) {
                ancestor[next][level] =
                    ancestor[ancestor[next][level - 1]][level - 1];
            }

            order.push_back(next);
            stack.push_back(next);
        }
    }

    auto findLCA = [&](int u, int v) {
        if (depth[u] < depth[v]) swap(u, v);

        int difference = depth[u] - depth[v];
        for (int level = 0; level < LOG; ++level) {
            if (difference & (1 << level)) u = ancestor[u][level];
        }

        if (u == v) return u;

        for (int level = LOG - 1; level >= 0; --level) {
            if (ancestor[u][level] != ancestor[v][level]) {
                u = ancestor[u][level];
                v = ancestor[v][level];
            }
        }
        return parent[u];
    };

    PersistentSegmentTree segmentTree(compressedCosts, M);
    vector<int> version(N, 0);

    for (int city : order) {
        if (city == 0) continue;

        version[city] = version[parent[city]];
        for (long long cost : edgeCosts[parentEdge[city]]) {
            int position = (int)(
                lower_bound(compressedCosts.begin(), compressedCosts.end(), cost)
                - compressedCosts.begin()
            );
            version[city] = segmentTree.update(
                version[city], 0, (int)compressedCosts.size() - 1, position, cost
            );
        }
    }

    while (Q--) {
        int source, target;
        long long gold, silver;
        cin >> source >> target >> gold >> silver;
        --source;
        --target;

        int lca = findLCA(source, target);
        long long checkpointCount =
            segmentTree.pathCount(version[source], version[target], version[lca]);

        long long affordableWithSilver = segmentTree.maximumAffordable(
            version[source],
            version[target],
            version[lca],
            0,
            (int)compressedCosts.size() - 1,
            silver
        );

        long long requiredGold = checkpointCount - affordableWithSilver;

        if (requiredGold > gold) {
            cout << -1 << '\n';
        } else {
            cout << gold - requiredGold << '\n';
        }
    }

    return 0;
}
