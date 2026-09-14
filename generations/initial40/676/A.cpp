#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdint>

using namespace std;

using int64 = long long;
using int128 = __int128_t;

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

    const int64 INF = (1LL << 60);
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

    // All-pairs shortest walking times.
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

    // Maximum profit from leaving i empty and next becoming empty at j.
    vector<vector<int64>> profit(N, vector<int64>(N, 0));

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            int64 best = 0;  // It is always possible to make no trade.
            for (int item = 0; item < K; ++item) {
                if (buy[i][item] != -1 && sell[j][item] != -1) {
                    best = max(best, sell[j][item] - buy[i][item]);
                }
            }
            profit[i][j] = best;
        }
    }

    auto possible = [&](int64 efficiency) -> bool {
        const int128 NEG = -((int128)1 << 120);
        vector<vector<int128>> best(N, vector<int128>(N, NEG));

        // Exclude i == j: that would be a zero-duration artificial edge.
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                if (i != j && dist[i][j] != INF) {
                    best[i][j] =
                        (int128)profit[i][j]
                        - (int128)efficiency * dist[i][j];
                }
            }
        }

        // Maximum-weight Floyd-Warshall.
        for (int k = 0; k < N; ++k) {
            for (int i = 0; i < N; ++i) {
                if (best[i][k] == NEG) continue;
                for (int j = 0; j < N; ++j) {
                    if (best[k][j] == NEG) continue;
                    best[i][j] = max(best[i][j],
                                     best[i][k] + best[k][j]);
                }
            }
        }

        for (int i = 0; i < N; ++i) {
            if (best[i][i] >= 0) {
                return true;
            }
        }
        return false;
    };

    int64 low = 0;
    int64 high = 1000000000LL;

    while (low < high) {
        int64 mid = low + (high - low + 1) / 2;
        if (possible(mid)) {
            low = mid;
        } else {
            high = mid - 1;
        }
    }

    cout << low << '\n';
    return 0;
}