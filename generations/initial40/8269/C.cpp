#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>

using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<vector<int>> graph(n);
    for (int i = 0; i < n - 1; ++i) {
        int u, v;
        cin >> u >> v;
        --u;
        --v;
        graph[u].push_back(v);
        graph[v].push_back(u);
    }

    vector<int> parent(n, -1), order;
    order.reserve(n);

    vector<int> stack;
    stack.push_back(0);
    parent[0] = -2;

    while (!stack.empty()) {
        int v = stack.back();
        stack.pop_back();
        order.push_back(v);

        for (int to : graph[v]) {
            if (to == parent[v]) continue;
            parent[to] = v;
            stack.push_back(to);
        }
    }

    vector<long long> subtree_size(n, 1);
    vector<long long> dp(n, 0);

    for (int i = n - 1; i >= 0; --i) {
        int v = order[i];
        long long previous_sizes = 0;

        for (int to : graph[v]) {
            if (parent[to] != v) continue;

            dp[v] += dp[to];
            dp[v] += subtree_size[to];
            dp[v] += 2LL * previous_sizes * subtree_size[to];

            previous_sizes += subtree_size[to];
            subtree_size[v] += subtree_size[to];
        }
    }

    vector<long long> answer(n);
    answer[0] = dp[0];

    long long best = answer[0];
    for (int v : order) {
        best = min(best, answer[v]);

        for (int to : graph[v]) {
            if (parent[to] != v) continue;

            answer[to] = answer[v] + 2LL * subtree_size[to] - n;
        }
    }

    for (int v = 0; v < n; ++v) {
        best = min(best, answer[v]);
    }

    cout << best << '\n';
    return 0;
}
