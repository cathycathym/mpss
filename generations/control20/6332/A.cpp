#include <bits/stdc++.h>
using namespace std;

struct Node {
    int left = 0;
    int right = 0;
    int count = 0;
    long long sum = 0;
};

struct Edge {
    int to;
    int id;
};

vector<Node> segtree(1);

int update(int previous, int low, int high, int position, long long value) {
    int current = (int)segtree.size();
    segtree.push_back(segtree[previous]);
    ++segtree[current].count;
    segtree[current].sum += value;

    if (low != high) {
        int middle = (low + high) / 2;
        if (position <= middle) {
            segtree[current].left =
                update(segtree[previous].left, low, middle, position, value);
        } else {
            segtree[current].right =
                update(segtree[previous].right, middle + 1, high, position, value);
        }
    }

    return current;
}

long long combinedSum(int a, int b, int c, int d) {
    return segtree[a].sum + segtree[b].sum
         - segtree[c].sum - segtree[d].sum;
}

int combinedCount(int a, int b, int c, int d) {
    return segtree[a].count + segtree[b].count
         - segtree[c].count - segtree[d].count;
}

long long maximumSilverPayments(
    int a, int b, int c, int d,
    int low, int high,
    long long &silver,
    const vector<long long> &values
) {
    int totalCount = combinedCount(a, b, c, d);
    if (totalCount == 0) return 0;

    if (low == high) {
        long long canPay = min<long long>(
            totalCount, silver / values[low]
        );
        silver -= canPay * values[low];
        return canPay;
    }

    int al = segtree[a].left;
    int bl = segtree[b].left;
    int cl = segtree[c].left;
    int dl = segtree[d].left;

    long long leftSum = combinedSum(al, bl, cl, dl);
    int middle = (low + high) / 2;

    if (leftSum > silver) {
        return maximumSilverPayments(
            al, bl, cl, dl, low, middle, silver, values
        );
    }

    int leftCount = combinedCount(al, bl, cl, dl);
    silver -= leftSum;

    return leftCount + maximumSilverPayments(
        segtree[a].right,
        segtree[b].right,
        segtree[c].right,
        segtree[d].right,
        middle + 1,
        high,
        silver,
        values
    );
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M, Q;
    cin >> N >> M >> Q;

    vector<vector<Edge>> graph(N + 1);
    for (int i = 1; i <= N - 1; ++i) {
        int a, b;
        cin >> a >> b;
        graph[a].push_back({b, i});
        graph[b].push_back({a, i});
    }

    vector<vector<long long>> edgeCosts(N);
    vector<long long> values;
    values.reserve(M);

    for (int i = 0; i < M; ++i) {
        int road;
        long long cost;
        cin >> road >> cost;
        edgeCosts[road].push_back(cost);
        values.push_back(cost);
    }

    sort(values.begin(), values.end());
    values.erase(unique(values.begin(), values.end()), values.end());

    int LOG = 1;
    while ((1 << LOG) <= N) ++LOG;

    vector<vector<int>> parent(LOG, vector<int>(N + 1, 0));
    vector<int> depth(N + 1, 0);
    vector<int> persistentRoot(N + 1, 0);

    segtree.reserve((size_t)M * 20 + 1);

    vector<int> order;
    order.reserve(N);
    order.push_back(1);

    for (size_t index = 0; index < order.size(); ++index) {
        int city = order[index];

        for (const Edge &edge : graph[city]) {
            if (edge.to == parent[0][city]) continue;

            int child = edge.to;
            parent[0][child] = city;
            depth[child] = depth[city] + 1;
            persistentRoot[child] = persistentRoot[city];

            for (long long cost : edgeCosts[edge.id]) {
                int position = lower_bound(values.begin(), values.end(), cost)
                             - values.begin();
                persistentRoot[child] = update(
                    persistentRoot[child],
                    0,
                    (int)values.size() - 1,
                    position,
                    cost
                );
            }

            order.push_back(child);
        }
    }

    for (int level = 1; level < LOG; ++level) {
        for (int city = 1; city <= N; ++city) {
            parent[level][city] =
                parent[level - 1][parent[level - 1][city]];
        }
    }

    auto lca = [&](int u, int v) {
        if (depth[u] < depth[v]) swap(u, v);

        int difference = depth[u] - depth[v];
        for (int level = 0; level < LOG; ++level) {
            if (difference & (1 << level)) {
                u = parent[level][u];
            }
        }

        if (u == v) return u;

        for (int level = LOG - 1; level >= 0; --level) {
            if (parent[level][u] != parent[level][v]) {
                u = parent[level][u];
                v = parent[level][v];
            }
        }

        return parent[0][u];
    };

    while (Q--) {
        int s, t;
        long long gold, silver;
        cin >> s >> t >> gold >> silver;

        int ancestor = lca(s, t);

        int a = persistentRoot[s];
        int b = persistentRoot[t];
        int c = persistentRoot[ancestor];
        int d = persistentRoot[ancestor];

        long long checkpointCount = combinedCount(a, b, c, d);
        long long remainingSilver = silver;

        long long paidWithSilver = maximumSilverPayments(
            a, b, c, d,
            0, (int)values.size() - 1,
            remainingSilver,
            values
        );

        long long requiredGold = checkpointCount - paidWithSilver;

        if (requiredGold > gold) {
            cout << -1 << '\n';
        } else {
            cout << gold - requiredGold << '\n';
        }
    }

    return 0;
}
