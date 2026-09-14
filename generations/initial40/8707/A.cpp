#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

struct Job {
    int64 profit;
    int64 need;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    int64 s;
    cin >> n >> s;

    vector<int64> x(n + 1), gain(n + 1), need(n + 1);
    vector<vector<int>> children(n + 1);
    vector<int> roots;

    for (int i = 1; i <= n; ++i) {
        int p;
        cin >> x[i] >> p;
        if (p == 0) roots.push_back(i);
        else children[p].push_back(i);
    }

    // gain[v] is the best total profit obtainable after deciding to do v.
    // A subtree with non-positive gain is never useful.
    for (int v = n; v >= 1; --v) {
        gain[v] = x[v];
        for (int u : children[v]) {
            if (gain[u] > 0) gain[v] += gain[u];
        }
    }

    // For every profitable subtree, calculate the minimum amount of money
    // needed before starting it. Its profitable child subtrees are processed
    // in increasing order of their required capital.
    for (int v = n; v >= 1; --v) {
        if (gain[v] <= 0) continue;

        vector<Job> useful;
        for (int u : children[v]) {
            if (gain[u] > 0) useful.push_back({gain[u], need[u]});
        }

        sort(useful.begin(), useful.end(), [](const Job& a, const Job& b) {
            return a.need < b.need;
        });

        int64 current = x[v];
        need[v] = max<int64>(0, -current);

        for (const Job& child : useful) {
            need[v] = max(need[v], child.need - current);
            current += child.profit;
        }
    }

    vector<Job> available;
    for (int r : roots) {
        if (gain[r] > 0) available.push_back({gain[r], need[r]});
    }

    sort(available.begin(), available.end(), [](const Job& a, const Job& b) {
        return a.need < b.need;
    });

    int64 money = s;
    for (const Job& job : available) {
        if (money < job.need) break;
        money += job.profit;
    }

    cout << money - s << '\n';
    return 0;
}
