#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <limits>
#include <utility>

using namespace std;

using int64 = long long;

struct Edge {
    int u, v;
    int64 w;
};

class DSU {
    vector<int> parent;
    vector<int> size_;

public:
    explicit DSU(int n) : parent(n), size_(n, 1) {
        for (int i = 0; i < n; ++i) parent[i] = i;
    }

    int find(int x) {
        int root = x;
        while (parent[root] != root) root = parent[root];
        while (parent[x] != x) {
            int next = parent[x];
            parent[x] = root;
            x = next;
        }
        return root;
    }

    void unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return;
        if (size_[a] < size_[b]) swap(a, b);
        parent[b] = a;
        size_[a] += size_[b];
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    vector<int> type(n);
    vector<int> hikers;
    for (int i = 0; i < n; ++i) {
        cin >> type[i];
        if (type[i] == 1) hikers.push_back(i);
    }

    vector<Edge> edges;
    edges.reserve(m);
    vector<vector<pair<int, int64>>> graph(n);

    for (int i = 0; i < m; ++i) {
        int a, b;
        int64 w;
        cin >> a >> b >> w;
        --a;
        --b;
        edges.push_back({a, b, w});
        graph[a].push_back({b, w});
        graph[b].push_back({a, w});
    }

    const int64 INF = numeric_limits<int64>::max() / 4;
    vector<int64> dist(n, INF);

    using State = pair<int64, int>;
    priority_queue<State, vector<State>, greater<State>> pq;

    for (int h : hikers) {
        dist[h] = 0;
        pq.push({0, h});
    }

    while (!pq.empty()) {
        int64 currentDistance = pq.top().first;
        int v = pq.top().second;
        pq.pop();

        if (currentDistance != dist[v]) continue;

        for (const auto& next : graph[v]) {
            int to = next.first;
            int64 weight = next.second;
            if (dist[to] > currentDistance + weight) {
                dist[to] = currentDistance + weight;
                pq.push({dist[to], to});
            }
        }
    }

    vector<int64> candidates;
    for (int i = 0; i < n; ++i) {
        if (type[i] == 0) candidates.push_back(dist[i]);
    }

    sort(candidates.begin(), candidates.end());
    candidates.erase(unique(candidates.begin(), candidates.end()),
                     candidates.end());

    auto feasible = [&](int64 threshold, vector<int>* answer) -> bool {
        vector<char> wallable(n, false);
        for (int i = 0; i < n; ++i) {
            wallable[i] = (type[i] == 0 && dist[i] <= threshold);
        }

        DSU dsu(n);

        for (const Edge& e : edges) {
            if (!wallable[e.u] && !wallable[e.v]) {
                dsu.unite(e.u, e.v);
            }
        }

        vector<char> bad(n, false);
        for (int i = 0; i < n; ++i) {
            if (type[i] == -1) {
                bad[dsu.find(i)] = true;
            }
        }

        for (int h : hikers) {
            if (bad[dsu.find(h)]) return false;
        }

        vector<char> blocked(n, false);

        for (const Edge& e : edges) {
            if (wallable[e.u] && !wallable[e.v] &&
                bad[dsu.find(e.v)]) {
                blocked[e.u] = true;
            }
            if (wallable[e.v] && !wallable[e.u] &&
                bad[dsu.find(e.u)]) {
                blocked[e.v] = true;
            }
        }

        auto isSafe = [&](int v) -> bool {
            if (wallable[v]) return !blocked[v];
            return !bad[dsu.find(v)];
        };

        vector<char> visited(n, false);
        queue<int> q;

        int start = hikers[0];
        visited[start] = true;
        q.push(start);

        while (!q.empty()) {
            int v = q.front();
            q.pop();

            for (const auto& next : graph[v]) {
                int to = next.first;
                if (!visited[to] && isSafe(to)) {
                    visited[to] = true;
                    q.push(to);
                }
            }
        }

        for (int h : hikers) {
            if (!visited[h]) return false;
        }

        if (answer != nullptr) {
            answer->clear();
            for (int i = 0; i < n; ++i) {
                if (wallable[i] && !visited[i]) {
                    answer->push_back(i);
                }
            }
        }

        return true;
    };

    if (candidates.empty() ||
        !feasible(candidates.back(), nullptr)) {
        cout << -1 << '\n';
        return 0;
    }

    int low = 0;
    int high = static_cast<int>(candidates.size()) - 1;

    while (low < high) {
        int mid = low + (high - low) / 2;
        if (feasible(candidates[mid], nullptr)) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }

    vector<int> answer;
    feasible(candidates[low], &answer);

    cout << answer.size() << '\n';
    for (int i = 0; i < static_cast<int>(answer.size()); ++i) {
        if (i > 0) cout << ' ';
        cout << answer[i] + 1;
    }
    cout << '\n';

    return 0;
}