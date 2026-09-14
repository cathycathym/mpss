#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

struct Package {
    long long need;
    long long gain;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    long long s;
    cin >> n >> s;

    vector<long long> x(n + 1), gain(n + 1, 0), need(n + 1, 0);
    vector<vector<int>> children(n + 1);
    vector<int> roots;

    for (int i = 1; i <= n; ++i) {
        int p;
        cin >> x[i] >> p;
        if (p == 0) roots.push_back(i);
        else children[p].push_back(i);
    }

    for (int v = n; v >= 1; --v) {
        vector<Package> useful;
        for (int u : children[v]) {
            if (gain[u] > 0) {
                useful.push_back({need[u], gain[u]});
            }
        }

        sort(useful.begin(), useful.end(),
             [](const Package& a, const Package& b) {
                 return a.need < b.need;
             });

        long long currentGain = x[v];
        long long required = max(0LL, -x[v]);

        for (const Package& p : useful) {
            required = max(required, p.need - currentGain);
            currentGain += p.gain;
        }

        gain[v] = currentGain;
        need[v] = required;
    }

    vector<Package> projects;
    for (int r : roots) {
        if (gain[r] > 0) {
            projects.push_back({need[r], gain[r]});
        }
    }

    sort(projects.begin(), projects.end(),
         [](const Package& a, const Package& b) {
             return a.need < b.need;
         });

    long long money = s;
    for (const Package& p : projects) {
        if (p.need > money) break;
        money += p.gain;
    }

    cout << money - s << '\n';
    return 0;
}
