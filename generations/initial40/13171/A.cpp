#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M, K;
    cin >> N >> M >> K;

    vector<vector<int>> score(N, vector<int>(M));
    vector<int> total(N, 0);
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < M; ++j) {
            cin >> score[i][j];
            total[i] += score[i][j];
        }
    }

    vector<int> order(N);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int x, int y) {
        if (total[x] != total[y]) return total[x] > total[y];
        return x < y;
    });

    const int MAXS = M * K;
    vector<int> slack(N - 1);
    for (int i = 0; i + 1 < N; ++i) {
        int x = order[i], y = order[i + 1];
        int needed = (x < y ? 0 : 1);
        slack[i] = total[x] - total[y] - needed;
    }

    const int NEG = -1000000000;
    vector<int> dp(1, 0);

    for (int pos = 0; pos < N; ++pos) {
        int id = order[pos];
        int leftCap = (pos == 0 ? MAXS : slack[pos - 1]);
        int rightCap = (pos + 1 == N ? MAXS : slack[pos]);

        vector<int> pref(leftCap + 1);
        pref[0] = dp[0];
        for (int x = 1; x <= leftCap; ++x)
            pref[x] = max(pref[x - 1], dp[x]);

        vector<int> ndp(rightCap + 1, NEG);

        // If no capacity exists on the right, only score-0 cells can be hidden.
        if (rightCap == 0) {
            int zeros = 0;
            for (int x : score[id]) if (x == 0) ++zeros;

            for (int c = 0; c <= zeros && K * c <= leftCap; ++c) {
                ndp[0] = max(ndp[0], pref[leftCap - K * c] + c);
            }
            dp.swap(ndp);
            continue;
        }

        // If no capacity exists on the left, only score-K cells can be hidden.
        if (leftCap == 0) {
            int full = 0;
            for (int x : score[id]) if (x == K) ++full;

            for (int c = 0; c <= full && K * c <= rightCap; ++c) {
                int r = K * c;
                ndp[r] = max(ndp[r], pref[0] + c);
            }
            dp.swap(ndp);
            continue;
        }

        int words = (rightCap + 64) / 64;
        vector<vector<unsigned long long>> can(M + 1,
                                                vector<unsigned long long>(words, 0));
        can[0][0] = 1ULL;

        int used = 0;
        for (int val : score[id]) {
            ++used;
            int shiftWords = val >> 6;
            int shiftBits = val & 63;

            for (int c = min(used, M); c >= 1; --c) {
                for (int w = words - 1; w >= 0; --w) {
                    unsigned long long add = 0;
                    int src = w - shiftWords;
                    if (src >= 0) {
                        add |= can[c - 1][src] << shiftBits;
                        if (shiftBits && src - 1 >= 0) {
                            add |= can[c - 1][src - 1] >> (64 - shiftBits);
                        }
                    }
                    can[c][w] |= add;
                }
            }
        }

        int validBits = (rightCap + 1) & 63;
        if (validBits != 0) {
            unsigned long long mask = (1ULL << validBits) - 1ULL;
            for (int c = 0; c <= M; ++c)
                can[c].back() &= mask;
        }

        for (int c = 0; c <= M; ++c) {
            for (int r = 0; r <= rightCap; ++r) {
                if (((can[c][r >> 6] >> (r & 63)) & 1ULL) == 0) continue;

                int leftUse = K * c - r;
                if (leftUse < 0 || leftUse > leftCap) continue;

                ndp[r] = max(ndp[r], pref[leftCap - leftUse] + c);
            }
        }

        dp.swap(ndp);
    }

    int maximumHidden = *max_element(dp.begin(), dp.end());
    cout << N * M - maximumHidden << '\n';
    return 0;
}
