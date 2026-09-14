#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdint>

using namespace std;

using int64 = long long;
using i128 = __int128_t;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M, K;
    cin >> N >> M >> K;

    vector<vector<int64>> buy(N, vector<int64>(K));
    vector<vector<int64>> sell(N, vector<int64>(K));

    for (int i = 0; i < N; ++i) {
        for (int k = 0; k < K; ++k) {
            cin >> buy[i][k] >> sell[i][k];
        }
    }

    const int64 INF = 4000000000000000000LL;
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

    vector<vector<int64>> gain(N, vector<int64>(N, 0));

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            if (i == j || dist[i][j] == INF) continue;

            int64 best = 0;
            for (int k = 0; k < K; ++k) {
                if (buy[i][k] != -1 && sell[j][k] != -1) {
                    best = max(best, sell[j][k] - buy[i][k]);
                }
            }
            gain[i][j] = best;
        }
    }

    auto feasible = [&](int64 efficiency) -> bool {
        const i128 NEG = -((i128)1 << 120);
        vector<vector<i128>> best(N, vector<i128>(N, NEG));

        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                if (i != j && dist[i][j] != INF) {
                    best[i][j] =
                        (i128)gain[i][j] -
                        (i128)efficiency * dist[i][j];
                }
            }
        }

        for (int k = 0; k < N; ++k) {
            for (int i = 0; i < N; ++i) {
                if (best[i][k] == NEG) continue;

                for (int j = 0; j < N; ++j) {
                    if (best[k][j] == NEG) continue;

                    i128 candidate = best[i][k] + best[k][j];
                    if (candidate > best[i][j]) {
                        best[i][j] = candidate;
                    }
                }
            }

            for (int i = 0; i < N; ++i) {
                if (best[i][i] >= 0) {
                    return true;
                }
            }
        }

        return false;
    };

    int64 low = 0;
    int64 high = 1000000001LL;

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