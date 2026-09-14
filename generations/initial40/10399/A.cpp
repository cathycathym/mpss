#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <limits>
#include <functional>
#include <utility>

using namespace std;

using ll = long long;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    vector<int> type(n);
    vector<int> hikers, cows;
    for (int i = 0; i < n; ++i) {
        cin >> type[i];
        if (type[i] == 1) hikers.push_back(i);
        if (type[i] == -1) cows.push_back(i);
    }

    vector<vector<pair<int, ll>>> graph(n);
    for (int i = 0; i < m; ++i) {
        int a, b;
        ll length;
        cin >> a >> b >> length;
        --a;
        --b;
        graph[a].push_back({b, length});
        graph[b].push_back({a, length});
    }

    // Multi-source Dijkstra from all hiking areas.
    const ll INF = numeric_limits<ll>::max() / 4;
    vector<ll> dist(n, INF);

    priority_queue<
        pair<ll, int>,
        vector<pair<ll, int>>,
        greater<pair<ll, int>>
    > pq;

    for (int h : hikers) {
        dist[h] = 0;
        pq.push({0, h});
    }

    while (!pq.empty()) {
        ll currentDist = pq.top().first;
        int v = pq.top().second;
        pq.pop();

        if (currentDist != dist[v]) continue;

        for (const auto &edge : graph[v]) {
            int to = edge.first;
            ll weight = edge.second;

            if (dist[to] > currentDist + weight) {
                dist[to] = currentDist + weight;
                pq.push({dist[to], to});
            }
        }
    }

    vector<ll> candidates;
    candidates.reserve(n);
    for (int v = 0; v < n; ++v) {
        if (type[v] == 0) candidates.push_back(dist[v]);
    }

    if (candidates.empty()) {
        cout << -1 << '\n';
        return 0;
    }

    sort(candidates.begin(), candidates.end());
    candidates.erase(unique(candidates.begin(), candidates.end()),
                     candidates.end());

    auto feasible = [&](ll limit, vector<int>* answer) -> bool {
        vector<char> eligible(n, false);
        for (int v = 0; v < n; ++v) {
            if (type[v] == 0 && dist[v] <= limit) {
                eligible[v] = true;
            }
        }

        // Reachability from cows without using any eligible vertex.
        vector<char> cowReachable(n, false);
        queue<int> q;

        for (int cow : cows) {
            cowReachable[cow] = true;
            q.push(cow);
        }

        while (!q.empty()) {
            int v = q.front();
            q.pop();

            for (const auto &edge : graph[v]) {
                int to = edge.first;
                if (!eligible[to] && !cowReachable[to]) {
                    cowReachable[to] = true;
                    q.push(to);
                }
            }
        }

        for (int h : hikers) {
            if (cowReachable[h]) return false;
        }

        // Canonical wall set: eligible vertices adjacent to cow-reachable
        // non-eligible territory.
        vector<char> isWall(n, false);
        vector<int> walls;

        for (int v = 0; v < n; ++v) {
            if (!cowReachable[v]) continue;

            for (const auto &edge : graph[v]) {
                int to = edge.first;
                if (eligible[to] && !isWall[to]) {
                    isWall[to] = true;
                    walls.push_back(to);
                }
            }
        }

        // Check that all hiking areas remain connected.
        vector<char> visited(n, false);
        queue<int> hq;

        int start = hikers[0];
        visited[start] = true;
        hq.push(start);

        while (!hq.empty()) {
            int v = hq.front();
            hq.pop();

            for (const auto &edge : graph[v]) {
                int to = edge.first;
                if (!isWall[to] && !visited[to]) {
                    visited[to] = true;
                    hq.push(to);
                }
            }
        }

        for (int h : hikers) {
            if (!visited[h]) return false;
        }

        if (answer != nullptr) {
            *answer = move(walls);
        }
        return true;
    };

    int low = 0;
    int high = static_cast<int>(candidates.size()) - 1;
    int best = -1;

    while (low <= high) {
        int middle = low + (high - low) / 2;

        if (feasible(candidates[middle], nullptr)) {
            best = middle;
            high = middle - 1;
        } else {
            low = middle + 1;
        }
    }

    if (best == -1) {
        cout << -1 << '\n';
        return 0;
    }

    vector<int> answer;
    feasible(candidates[best], &answer);

    cout << answer.size() << '\n';
    for (size_t i = 0; i < answer.size(); ++i) {
        if (i) cout << ' ';
        cout << answer[i] + 1;
    }
    cout << '\n';

    return 0;
}