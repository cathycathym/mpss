#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<vector<int>> graph(n);
    for (int i = 1; i < n; ++i) {
        int u, v;
        cin >> u >> v;
        --u;
        --v;
        graph[u].push_back(v);
        graph[v].push_back(u);
    }

    vector<int> parent(n, -1), order;
    order.reserve(n);
    order.push_back(0);
    parent[0] = 0;

    for (int i = 0; i < n; ++i) {
        int v = order[i];
        for (int to : graph[v]) {
            if (to == parent[v]) continue;
            parent[to] = v;
            order.push_back(to);
        }
    }

    vector<long long> subtreeSize(n, 1);
    vector<long long> cost(n, 0);

    for (int i = n - 1; i >= 0; --i) {
        int v = order[i];
        long long processedSize = 0;

        for (int to : graph[v]) {
            if (parent[to] != v) continue;

            cost[v] += cost[to] + subtreeSize[to];
            cost[v] += 2LL * processedSize * subtreeSize[to];

            processedSize += subtreeSize[to];
            subtreeSize[v] += subtreeSize[to];
        }
    }

    vector<long long> rootCost(n);
    rootCost[0] = cost[0];

    long long answer = rootCost[0];

    for (int v : order) {
        answer = min(answer, rootCost[v]);

        for (int to : graph[v]) {
            if (parent[to] != v) continue;

            rootCost[to] =
                rootCost[v] + 2LL * subtreeSize[to] - n;
        }
    }

    cout << answer << '\n';
    return 0;
}