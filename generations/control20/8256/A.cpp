#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
const int64 INF = (int64)4e18;

struct Edge {
    int to;
    int64 cost;
};

vector<int64> dijkstra(int start, const vector<vector<Edge>>& graph) {
    int n = (int)graph.size();
    vector<int64> dist(n, INF);

    priority_queue<
        pair<int64, int>,
        vector<pair<int64, int>>,
        greater<pair<int64, int>>
    > pq;

    dist[start] = 0;
    pq.push({0, start});

    while (!pq.empty()) {
        auto [current_dist, vertex] = pq.top();
        pq.pop();

        if (current_dist != dist[vertex]) {
            continue;
        }

        for (const Edge& edge : graph[vertex]) {
            int64 next_dist = current_dist + edge.cost;
            if (next_dist < dist[edge.to]) {
                dist[edge.to] = next_dist;
                pq.push({next_dist, edge.to});
            }
        }
    }

    return dist;
}

class FenwickTree {
private:
    int size;
    vector<int64> tree;

public:
    explicit FenwickTree(int n) : size(n), tree(n + 1, 0) {}

    void add(int position, int64 value) {
        for (int i = position; i <= size; i += i & -i) {
            tree[i] += value;
        }
    }

    int64 prefix_sum(int position) const {
        int64 result = 0;
        for (int i = position; i > 0; i -= i & -i) {
            result += tree[i];
        }
        return result;
    }
};

struct Station {
    int64 from_s;
    int64 to_t;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M;
    cin >> N >> M;

    int S, T;
    int64 L, K;
    cin >> S >> T >> L >> K;
    --S;
    --T;

    vector<vector<Edge>> graph(N);

    for (int i = 0; i < M; ++i) {
        int A, B;
        int64 C;
        cin >> A >> B >> C;
        --A;
        --B;

        graph[A].push_back({B, C});
        graph[B].push_back({A, C});
    }

    vector<int64> dist_s = dijkstra(S, graph);
    vector<int64> dist_t = dijkstra(T, graph);

    // The already existing route may still be used after construction.
    if (dist_s[T] <= K) {
        cout << (int64)N * (N - 1) / 2 << '\n';
        return 0;
    }

    int64 limit = K - L;
    if (limit < 0) {
        cout << 0 << '\n';
        return 0;
    }

    vector<Station> stations;
    stations.reserve(N);

    for (int i = 0; i < N; ++i) {
        // A station unreachable from both S and T cannot participate in
        // a successful route using only one new edge.
        if (dist_s[i] == INF && dist_t[i] == INF) {
            continue;
        }
        stations.push_back({dist_s[i], dist_t[i]});
    }

    /*
    For stations x and y, the two possible routes using the new edge are

        dist_s[x] + L + dist_t[y]
        dist_s[y] + L + dist_t[x].

    Their difference, excluding L, is

        (dist_s[x] - dist_t[x]) - (dist_s[y] - dist_t[y]).

    Therefore, after sorting by dist_s[v] - dist_t[v], for an earlier
    station x and a later station y, the first route is never longer.
    The pair is successful exactly when

        dist_s[x] + dist_t[y] <= K - L.
    */
    sort(stations.begin(), stations.end(),
         [](const Station& x, const Station& y) {
             __int128 key_x = (__int128)x.from_s - x.to_t;
             __int128 key_y = (__int128)y.from_s - y.to_t;
             return key_x < key_y;
         });

    vector<int64> coordinates;
    coordinates.reserve(stations.size());
    for (const Station& station : stations) {
        coordinates.push_back(station.from_s);
    }

    sort(coordinates.begin(), coordinates.end());
    coordinates.erase(
        unique(coordinates.begin(), coordinates.end()),
        coordinates.end()
    );

    FenwickTree fenwick((int)coordinates.size());
    int64 answer = 0;

    for (const Station& station : stations) {
        int64 threshold = limit - station.to_t;

        int count_positions = (int)(
            upper_bound(coordinates.begin(), coordinates.end(), threshold)
            - coordinates.begin()
        );

        answer += fenwick.prefix_sum(count_positions);

        int position = (int)(
            lower_bound(
                coordinates.begin(),
                coordinates.end(),
                station.from_s
            ) - coordinates.begin()
        ) + 1;

        fenwick.add(position, 1);
    }

    cout << answer << '\n';
    return 0;
}
