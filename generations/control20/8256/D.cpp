#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

const int64 INF = 4'000'000'000'000'000'000LL;

struct Edge {
    int to;
    int64 cost;
};

vector<int64> dijkstra(int source, const vector<vector<Edge> >& graph) {
    const int n = static_cast<int>(graph.size());
    vector<int64> dist(n, INF);

    priority_queue<
        pair<int64, int>,
        vector<pair<int64, int> >,
        greater<pair<int64, int> >
    > pq;

    dist[source] = 0;
    pq.push(make_pair(0, source));

    while (!pq.empty()) {
        const pair<int64, int> current = pq.top();
        pq.pop();

        const int64 currentDist = current.first;
        const int vertex = current.second;

        if (currentDist != dist[vertex]) {
            continue;
        }

        for (const Edge& edge : graph[vertex]) {
            const int64 nextDist = currentDist + edge.cost;
            if (nextDist < dist[edge.to]) {
                dist[edge.to] = nextDist;
                pq.push(make_pair(nextDist, edge.to));
            }
        }
    }

    return dist;
}

class FenwickTree {
public:
    explicit FenwickTree(int size) : tree(size + 1, 0) {}

    void add(int index, int value) {
        for (int i = index; i < static_cast<int>(tree.size()); i += i & -i) {
            tree[i] += value;
        }
    }

    int64 prefixSum(int index) const {
        int64 result = 0;
        for (int i = index; i > 0; i -= i & -i) {
            result += tree[i];
        }
        return result;
    }

private:
    vector<int> tree;
};

struct Station {
    int64 distS;
    int64 distT;
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

    vector<vector<Edge> > graph(N);

    for (int i = 0; i < M; ++i) {
        int A, B;
        int64 C;
        cin >> A >> B >> C;
        --A;
        --B;

        graph[A].push_back(Edge{B, C});
        graph[B].push_back(Edge{A, C});
    }

    const vector<int64> distS = dijkstra(S, graph);
    const vector<int64> distT = dijkstra(T, graph);

    if (distS[T] <= K) {
        cout << static_cast<int64>(N) * (N - 1) / 2 << '\n';
        return 0;
    }

    const int64 limit = K - L;
    if (limit < 0) {
        cout << 0 << '\n';
        return 0;
    }

    int64 answer = 0;

    if (distS[T] == INF) {
        vector<int64> fromS;
        vector<int64> toT;

        for (int i = 0; i < N; ++i) {
            if (distS[i] != INF) {
                fromS.push_back(distS[i]);
            }
            if (distT[i] != INF) {
                toT.push_back(distT[i]);
            }
        }

        sort(toT.begin(), toT.end());

        for (int64 distance : fromS) {
            answer += upper_bound(
                toT.begin(), toT.end(), limit - distance
            ) - toT.begin();
        }
    } else {
        vector<Station> stations;

        for (int i = 0; i < N; ++i) {
            if (distS[i] != INF) {
                stations.push_back(Station{distS[i], distT[i]});
            }
        }

        sort(stations.begin(), stations.end(),
             [](const Station& a, const Station& b) {
                 return a.distS - a.distT < b.distS - b.distT;
             });

        vector<int64> coordinates;
        coordinates.reserve(stations.size());

        for (const Station& station : stations) {
            coordinates.push_back(station.distS);
        }

        sort(coordinates.begin(), coordinates.end());
        coordinates.erase(
            unique(coordinates.begin(), coordinates.end()),
            coordinates.end()
        );

        FenwickTree fenwick(static_cast<int>(coordinates.size()));

        for (const Station& station : stations) {
            const int64 threshold = limit - station.distT;
            const int compressedLimit = static_cast<int>(
                upper_bound(
                    coordinates.begin(), coordinates.end(), threshold
                ) - coordinates.begin()
            );

            answer += fenwick.prefixSum(compressedLimit);

            const int position = static_cast<int>(
                lower_bound(
                    coordinates.begin(), coordinates.end(), station.distS
                ) - coordinates.begin()
            ) + 1;

            fenwick.add(position, 1);
        }
    }

    cout << answer << '\n';
    return 0;
}
