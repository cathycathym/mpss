#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdint>

using namespace std;

using int64 = long long;
using i128 = __int128_t;

struct Edge {
    int from, to;
    int64 distance;
    int64 profit;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M, K;
    cin >> N >> M >> K;

    vector<vector<int64>> buy(N, vector<int64>(K));
    vector<vector<int64>> sell(N, vector<int64>(K));

    for (int i = 0; i < N; ++i) {
        for (int item = 0; item < K; ++item) {
            cin >> buy[i][item] >> sell[i][item];
        }
    }

    const int64 INF = (1LL << 62);
    vector<vector<int64>> dist(N, vector<int64>(N, INF));

    for (int i = 0; i < N; ++i) {
        dist[i][i] = 0;
    }

    for (int e = 0; e < M; ++e) {
        int v, w;
        int64 t;
        cin >> v >> w >> t;
        --v;
        --w;
        dist[v][w] = t;
    }

    for (int k = 0; k < N; ++k) {
        for (int i = 0; i < N; ++i) {
            if (dist[i][k] == INF) continue;
            for (int j = 0; j < N; ++j) {
                if (dist[k][j] == INF) continue;
                dist[i][j] = min(dist[i][j],
                                 dist[i][k] + dist[k][j]);
            }
        }
    }

    vector<vector<int64>> profit(N, vector<int64>(N, 0));

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            int64 best = 0;
            for (int item = 0; item < K; ++item) {
                if (buy[i][item] != -1 && sell[j][item] != -1) {
                    best = max(best, sell[j][item] - buy[i][item]);
                }
            }
            profit[i][j] = best;
        }
    }

    vector<Edge> edges;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            if (i != j && dist[i][j] != INF) {
                edges.push_back({i, j, dist[i][j], profit[i][j]});
            }
        }
    }

    auto feasible = [&](int64 efficiency) {
        /*
         * An edge has adjusted weight
         *     profit - efficiency * time.
         *
         * We need a cycle whose adjusted weight is at least zero.
         * Since adjusted weights are integral, multiply each weight by
         * N+1 and add 1 per edge. For every simple cycle (length <= N),
         * the resulting weight is positive exactly when the original
         * adjusted weight is nonnegative.
         */
        const i128 SCALE = N + 1;
        vector<i128> best(N, 0);

        for (int iteration = 0; iteration < N; ++iteration) {
            vector<i128> next = best;
            bool changed = false;

            for (const Edge& edge : edges) {
                i128 adjusted =
                    (i128)edge.profit -
                    (i128)efficiency * edge.distance;
                i128 weight = adjusted * SCALE + 1;
                i128 candidate = best[edge.from] + weight;

                if (candidate > next[edge.to]) {
                    next[edge.to] = candidate;
                    changed = true;
                }
            }

            best.swap(next);

            if (!changed) {
                return false;
            }
        }

        return true;
    };

    int64 low = 0;
    int64 high = 1000000000LL;

    while (low + 1 < high) {
        int64 mid = low + (high - low) / 2;
        if (feasible(mid)) {
            low = mid;
        } else {
            high = mid;
        }
    }

    cout << low << '\n';
    return 0;
}