#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <limits>
#include <functional>
#include <utility>

using namespace std;

using int64 = long long;

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

    vector<vector<pair<int, int64>>> graph(n);

    for (int i = 0; i < m; ++i) {
        int a, b;
        int64 length;
        cin >> a >> b >> length;
        --a;
        --b;
        graph[a].push_back({b, length});
        graph[b].push_back({a, length});
    }

    const int64 INF = numeric_limits<int64>::max() / 4;

    // Compute every area's remoteness using multi-source Dijkstra.
    vector<int64> distance(n, INF);
    priority_queue<
        pair<int64, int>,
        vector<pair<int64, int>>,
        greater<pair<int64, int>>
    > pq;

    for (int h : hikers) {
        distance[h] = 0;
        pq.push({0, h});
    }

    while (!pq.empty()) {
        int64 currentDistance = pq.top().first;
        int vertex = pq.top().second;
        pq.pop();

        if (currentDistance != distance[vertex]) continue;

        for (const auto& edge : graph[vertex]) {
            int next = edge.first;
            int64 weight = edge.second;

            if (distance[next] > currentDistance + weight) {
                distance[next] = currentDistance + weight;
                pq.push({distance[next], next});
            }
        }
    }

    vector<int64> thresholds;
    thresholds.reserve(n);

    for (int vertex = 0; vertex < n; ++vertex) {
        if (type[vertex] == 0) {
            thresholds.push_back(distance[vertex]);
        }
    }

    sort(thresholds.begin(), thresholds.end());
    thresholds.erase(
        unique(thresholds.begin(), thresholds.end()),
        thresholds.end()
    );

    auto feasible = [&](int64 threshold, vector<int>* result) -> bool {
        vector<char> available(n, false);

        for (int vertex = 0; vertex < n; ++vertex) {
            if (type[vertex] == 0 && distance[vertex] <= threshold) {
                available[vertex] = true;
            }
        }

        // R consists of all vertices reachable from a cow without entering
        // any currently available wall location.
        vector<char> cowReachable(n, false);
        queue<int> queue;

        for (int cow : cows) {
            cowReachable[cow] = true;
            queue.push(cow);
        }

        while (!queue.empty()) {
            int vertex = queue.front();
            queue.pop();

            for (const auto& edge : graph[vertex]) {
                int next = edge.first;
                if (!available[next] && !cowReachable[next]) {
                    cowReachable[next] = true;
                    queue.push(next);
                }
            }
        }

        // A cow-to-hiker path containing no available vertex cannot be cut.
        for (int hiker : hikers) {
            if (cowReachable[hiker]) return false;
        }

        // Wall off exactly the available vertices on the boundary of R.
        vector<char> isWall(n, false);
        vector<int> walls;

        for (int vertex = 0; vertex < n; ++vertex) {
            if (!cowReachable[vertex]) continue;

            for (const auto& edge : graph[vertex]) {
                int next = edge.first;
                if (available[next] && !isWall[next]) {
                    isWall[next] = true;
                    walls.push_back(next);
                }
            }
        }

        // Verify that the canonical boundary cut preserves connectivity
        // between all hiking areas.
        vector<char> visited(n, false);
        queue.push(hikers.front());
        visited[hikers.front()] = true;

        while (!queue.empty()) {
            int vertex = queue.front();
            queue.pop();

            for (const auto& edge : graph[vertex]) {
                int next = edge.first;
                if (!isWall[next] && !visited[next]) {
                    visited[next] = true;
                    queue.push(next);
                }
            }
        }

        for (int hiker : hikers) {
            if (!visited[hiker]) return false;
        }

        if (result != nullptr) {
            *result = move(walls);
        }

        return true;
    };

    if (thresholds.empty()) {
        cout << -1 << '\n';
        return 0;
    }

    int low = 0;
    int high = static_cast<int>(thresholds.size()) - 1;
    int best = -1;

    while (low <= high) {
        int middle = low + (high - low) / 2;

        if (feasible(thresholds[middle], nullptr)) {
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
    feasible(thresholds[best], &answer);

    cout << answer.size() << '\n';
    for (int i = 0; i < static_cast<int>(answer.size()); ++i) {
        if (i != 0) cout << ' ';
        cout << answer[i] + 1;
    }
    cout << '\n';

    return 0;
}