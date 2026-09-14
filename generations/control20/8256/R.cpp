#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

const int64 INF = (1LL << 62);

struct Edge {
    int to;
    int64 cost;
};

struct FenwickTree {
    int n;
    vector<int> tree;

    explicit FenwickTree(int size) : n(size), tree(size + 1, 0) {}

    void add(int index, int value) {
        for (; index <= n; index += index & -index) {
            tree[index] += value;
        }
    }

    int sumPrefix(int index) const {
        int result = 0;
        for (; index > 0; index -= index & -index) {
            result += tree[index];
        }
        return result;
    }
};

vector<int64> dijkstra(int start, const vector<vector<Edge>>& graph) {
    int n = static_cast<int>(graph.size());
    vector<int64> dist(n, INF);

    priority_queue<
        pair<int64, int>,
        vector<pair<int64, int>>,
        greater<pair<int64, int>>
    > pq;

    dist[start] = 0;
    pq.push({0, start});

    while (!pq.empty()) {
        auto [currentDist, vertex] = pq.top();
        pq.pop();

        if (currentDist != dist[vertex]) {
            continue;
        }

        for (const Edge& edge : graph[vertex]) {
            int64 nextDist = currentDist + edge.cost;
            if (nextDist < dist[edge.to]) {
                dist[edge.to] = nextDist;
                pq.push({nextDist, edge.to});
            }
        }
    }

    return dist;
}

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

    vector<int64> distS = dijkstra(S, graph);
    vector<int64> distT = dijkstra(T, graph);

    if (distS[T] <= K) {
        cout << 1LL * N * (N - 1) / 2 << '\n';
        return 0;
    }

    if (L > K) {
        cout << 0 << '\n';
        return 0;
    }

    int64 limit = K - L;

    vector<int> vertices;
    vertices.reserve(N);

    for (int i = 0; i < N; ++i) {
        if (distS[i] != INF || distT[i] != INF) {
            vertices.push_back(i);
        }
    }

    auto category = [&](int vertex) {
        bool reachableFromS = distS[vertex] != INF;
        bool reachableToT = distT[vertex] != INF;

        if (reachableFromS && !reachableToT) {
            return 0;
        }
        if (reachableFromS && reachableToT) {
            return 1;
        }
        return 2;
    };

    sort(vertices.begin(), vertices.end(), [&](int x, int y) {
        int categoryX = category(x);
        int categoryY = category(y);

        if (categoryX != categoryY) {
            return categoryX < categoryY;
        }

        if (categoryX == 1) {
            int64 differenceX = distS[x] - distT[x];
            int64 differenceY = distS[y] - distT[y];

            if (differenceX != differenceY) {
                return differenceX < differenceY;
            }
        }

        return x < y;
    });

    vector<int64> sourceDistances;
    sourceDistances.reserve(N);

    for (int vertex : vertices) {
        if (distS[vertex] != INF) {
            sourceDistances.push_back(distS[vertex]);
        }
    }

    sort(sourceDistances.begin(), sourceDistances.end());
    sourceDistances.erase(
        unique(sourceDistances.begin(), sourceDistances.end()),
        sourceDistances.end()
    );

    FenwickTree fenwick(static_cast<int>(sourceDistances.size()));
    int64 answer = 0;

    for (int vertex : vertices) {
        if (distT[vertex] != INF) {
            int64 maximumSourceDistance = limit - distT[vertex];
            int countCoordinates = static_cast<int>(
                upper_bound(
                    sourceDistances.begin(),
                    sourceDistances.end(),
                    maximumSourceDistance
                ) - sourceDistances.begin()
            );

            answer += fenwick.sumPrefix(countCoordinates);
        }

        if (distS[vertex] != INF) {
            int position = static_cast<int>(
                lower_bound(
                    sourceDistances.begin(),
                    sourceDistances.end(),
                    distS[vertex]
                ) - sourceDistances.begin()
            ) + 1;

            fenwick.add(position, 1);
        }
    }

    cout << answer << '\n';
    return 0;
}
