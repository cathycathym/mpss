#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<vector<int>> graph(n);
    for (int i = 0; i + 1 < n; ++i) {
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

    for (int i = 0; i < (int)order.size(); ++i) {
        int v = order[i];
        for (int to : graph[v]) {
            if (to == parent[v]) continue;
            parent[to] = v;
            order.push_back(to);
        }
    }

    vector<long long> subtree_size(n, 1), down(n, 0), up(n, 0);

    // down[v]: cost of serving v's rooted subtree when entering v from its parent.
    for (int i = n - 1; i >= 0; --i) {
        int v = order[i];
        long long sum_cost = 0;
        long long sum_squares = 0;

        for (int to : graph[v]) {
            if (parent[to] != v) continue;
            subtree_size[v] += subtree_size[to];
            sum_cost += down[to];
            sum_squares += subtree_size[to] * subtree_size[to];
        }

        long long s = subtree_size[v];
        down[v] = s + sum_cost + (s - 1) * (s - 1) - sum_squares;
    }

    long long answer = LLONG_MAX;

    for (int v : order) {
        long long total_cost = 0;
        long long total_squares = 0;

        for (int to : graph[v]) {
            if (parent[to] == v) {
                total_cost += down[to];
                total_squares += subtree_size[to] * subtree_size[to];
            } else if (parent[v] == to) {
                long long s = n - subtree_size[v];
                total_cost += up[v];
                total_squares += s * s;
            }
        }

        // Cost when starting directly in v.
        long long current = total_cost + 1LL * (n - 1) * (n - 1) - total_squares;
        answer = min(answer, current);

        // Compute the message from v to each child.
        for (int to : graph[v]) {
            if (parent[to] != v) continue;

            long long child_size = subtree_size[to];
            long long component_size = n - child_size;

            up[to] = component_size
                   + (total_cost - down[to])
                   + (component_size - 1) * (component_size - 1)
                   - (total_squares - child_size * child_size);
        }
    }

    cout << answer << '\n';
    return 0;
}
