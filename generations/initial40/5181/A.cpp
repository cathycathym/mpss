#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

class Fenwick {
    int n;
    vector<int64> bit;

public:
    explicit Fenwick(int n) : n(n), bit(n + 1, 0) {}

    void add(int index, int64 value) {
        for (int i = index; i <= n; i += i & -i)
            bit[i] += value;
    }

    void rangeAdd(int left, int right, int64 value) {
        if (left > right || value == 0) return;
        add(left, value);
        if (right + 1 <= n)
            add(right + 1, -value);
    }

    int64 pointQuery(int index) const {
        int64 result = 0;
        for (int i = index; i > 0; i -= i & -i)
            result += bit[i];
        return result;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m, q;
    cin >> n >> m >> q;

    vector<int> initialCity(m + 1);
    for (int i = 1; i <= m; ++i)
        cin >> initialCity[i];

    vector<vector<int>> graph(n + 1);
    for (int i = 0; i < n - 1; ++i) {
        int v, w;
        cin >> v >> w;
        graph[v].push_back(w);
        graph[w].push_back(v);
    }

    const int LOG = 19;
    vector<array<int, LOG>> up(n + 1);
    vector<int> depth(n + 1, 0);
    vector<int> order;
    order.reserve(n);

    up[1][0] = 1;
    order.push_back(1);

    for (size_t index = 0; index < order.size(); ++index) {
        int v = order[index];
        for (int to : graph[v]) {
            if (to == up[v][0]) continue;
            up[to][0] = v;
            depth[to] = depth[v] + 1;
            order.push_back(to);
        }
    }

    for (int level = 1; level < LOG; ++level) {
        for (int v = 1; v <= n; ++v)
            up[v][level] = up[up[v][level - 1]][level - 1];
    }

    auto lca = [&](int a, int b) {
        if (depth[a] < depth[b])
            swap(a, b);

        int difference = depth[a] - depth[b];
        for (int level = 0; level < LOG; ++level) {
            if (difference & (1 << level))
                a = up[a][level];
        }

        if (a == b) return a;

        for (int level = LOG - 1; level >= 0; --level) {
            if (up[a][level] != up[b][level]) {
                a = up[a][level];
                b = up[b][level];
            }
        }

        return up[a][0];
    };

    auto distance = [&](int a, int b) -> int64 {
        int ancestor = lca(a, b);
        return static_cast<int64>(depth[a]) + depth[b]
             - 2LL * depth[ancestor];
    };

    // Each entry (start, city) represents an interval that continues
    // until just before the next entry.
    map<int, int> intervals;
    intervals[1] = initialCity[1];
    for (int i = 2; i <= m; ++i) {
        if (initialCity[i] != initialCity[i - 1])
            intervals[i] = initialCity[i];
    }
    intervals[m + 1] = 0; // Sentinel.

    auto split = [&](int position) -> map<int, int>::iterator {
        auto existing = intervals.lower_bound(position);
        if (existing != intervals.end() && existing->first == position)
            return existing;

        auto containing = prev(existing);
        return intervals.emplace(position, containing->second).first;
    };

    Fenwick adjustedOpinion(m);
    vector<int64> eventSum(n + 1, 0);

    while (q--) {
        char type;
        cin >> type;

        if (type == 'e') {
            int city;
            int64 value;
            cin >> city >> value;
            eventSum[city] += value;
        } else if (type == 'q') {
            int tourist;
            cin >> tourist;

            auto interval = prev(intervals.upper_bound(tourist));
            int city = interval->second;

            cout << adjustedOpinion.pointQuery(tourist)
                 + eventSum[city] << '\n';
        } else {
            int left, right, destination;
            cin >> left >> right >> destination;

            auto after = split(right + 1);
            auto first = split(left);

            for (auto it = first; it != after; ++it) {
                auto nextIt = next(it);
                int segmentLeft = it->first;
                int segmentRight = nextIt->first - 1;
                int oldCity = it->second;

                int64 change =
                    eventSum[oldCity]
                    - distance(oldCity, destination)
                    - eventSum[destination];

                adjustedOpinion.rangeAdd(
                    segmentLeft, segmentRight, change
                );
            }

            intervals.erase(first, after);
            auto inserted = intervals.emplace(left, destination).first;

            // Merge with the preceding interval when possible.
            if (inserted != intervals.begin()) {
                auto previous = prev(inserted);
                if (previous->second == inserted->second) {
                    intervals.erase(inserted);
                    inserted = previous;
                }
            }

            // Merge with the following interval when possible.
            auto following = next(inserted);
            if (following != intervals.end() &&
                following->first != m + 1 &&
                following->second == inserted->second) {
                intervals.erase(following);
            }
        }
    }

    return 0;
}