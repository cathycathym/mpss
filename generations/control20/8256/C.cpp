#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <queue>
#include <tuple>
#include <utility>
#include <vector>

using namespace std;

using int64 = long long;

const int64 INF = 4'000'000'000'000'000'000LL;

struct Fenwick {
    int n;
    vector<int> tree;

    explicit Fenwick(int n_) : n(n_), tree(n_ + 1, 0) {}

    void add(int index, int value) {
        for (int i = index; i <= n; i += i & -i) {
            tree[i] += value;
        }
    }

    int sumPrefix(int index) const {
        int result = 0;
        for (int i = index; i > 0; i -= i & -i) {
            result += tree[i];
        }
        return result;
    }
};

struct Vertex {
    int64 ds;
    int64 dt;
    int type;
    int64 delta;
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

    vector<vector<pair<int, int64>>> graph(N);

    for (int i = 0; i < M; ++i) {
        int a, b;
        int64 c;
        cin >> a >> b >> c;
        --a;
        --b;
        graph[a].push_back({b, c});
        graph[b].push_back({a, c});
    }

    auto dijkstra = [&](int source) {
        vector<int64> dist(N, INF);
        priority_queue<
            pair<int64, int>,
            vector<pair<int64, int>>,
            greater<pair<int64, int>>
        > pq;

        dist[source] = 0;
        pq.push({0, source});

        while (!pq.empty()) {
            auto [currentDistance, vertex] = pq.top();
            pq.pop();

            if (currentDistance != dist[vertex]) {
                continue;
            }

            for (auto [next, weight] : graph[vertex]) {
                if (dist[next] > currentDistance + weight) {
                    dist[next] = currentDistance + weight;
                    pq.push({dist[next], next});
                }
            }
        }

        return dist;
    };

    vector<int64> distS = dijkstra(S);
    vector<int64> distT = dijkstra(T);

    if (distS[T] <= K) {
        cout << 1LL * N * (N - 1) / 2 << '\n';
        return 0;
    }

    const int64 D = K - L;
    if (D < 0) {
        cout << 0 << '\n';
        return 0;
    }

    vector<Vertex> vertices;
    vector<int64> coordinates;
    vertices.reserve(N);
    coordinates.reserve(N);

    for (int i = 0; i < N; ++i) {
        bool reachableFromS = (distS[i] != INF);
        bool reachableFromT = (distT[i] != INF);

        if (!reachableFromS && !reachableFromT) {
            continue;
        }

        if (reachableFromS) {
            coordinates.push_back(distS[i]);
        }

        if (reachableFromS && !reachableFromT) {
            vertices.push_back({distS[i], distT[i], 0, 0});
        } else if (reachableFromS && reachableFromT) {
            vertices.push_back({
                distS[i],
                distT[i],
                1,
                distS[i] - distT[i]
            });
        } else {
            vertices.push_back({distS[i], distT[i], 2, 0});
        }
    }

    sort(vertices.begin(), vertices.end(),
         [](const Vertex& a, const Vertex& b) {
             if (a.type != b.type) {
                 return a.type < b.type;
             }
             if (a.type == 1 && a.delta != b.delta) {
                 return a.delta < b.delta;
             }
             return false;
         });

    sort(coordinates.begin(), coordinates.end());
    coordinates.erase(
        unique(coordinates.begin(), coordinates.end()),
        coordinates.end()
    );

    Fenwick fenwick(static_cast<int>(coordinates.size()));
    int64 answer = 0;

    for (const Vertex& vertex : vertices) {
        if (vertex.dt != INF) {
            int64 limit = D - vertex.dt;
            int count = static_cast<int>(
                upper_bound(coordinates.begin(), coordinates.end(), limit)
                - coordinates.begin()
            );
            answer += fenwick.sumPrefix(count);
        }

        if (vertex.ds != INF) {
            int position = static_cast<int>(
                lower_bound(
                    coordinates.begin(),
                    coordinates.end(),
                    vertex.ds
                ) - coordinates.begin()
            ) + 1;
            fenwick.add(position, 1);
        }
    }

    cout << answer << '\n';
    return 0;
}
