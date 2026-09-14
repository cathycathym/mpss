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
        for (int &x : score[i]) {
            cin >> x;
            total[i] += x;
        }
    }

    vector<int> order(N);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int x, int y) {
        if (total[x] != total[y]) return total[x] > total[y];
        return x < y;
    });

    vector<int> cap(N - 1);
    for (int i = 0; i + 1 < N; ++i) {
        int higher = order[i];
        int lower = order[i + 1];
        int strict = (higher > lower ? 1 : 0);
        cap[i] = total[higher] - total[lower] - strict;
    }

    const int NEG = -1000000000;

    auto subsetReachability = [&](const vector<int>& values, int limit) {
        int words = limit / 64 + 1;
        vector<vector<uint64_t>> reach(
            M + 1, vector<uint64_t>(words, 0)
        );
        reach[0][0] = 1ULL;

        int used = 0;
        for (int value : values) {
            ++used;
            int wordShift = value / 64;
            int bitShift = value % 64;

            for (int cnt = used; cnt >= 1; --cnt) {
                for (int w = words - 1; w >= 0; --w) {
                    uint64_t add = 0;
                    int src = w - wordShift;

                    if (src >= 0) {
                        add |= reach[cnt - 1][src] << bitShift;
                        if (bitShift != 0 && src - 1 >= 0) {
                            add |= reach[cnt - 1][src - 1]
                                   >> (64 - bitShift);
                        }
                    }
                    reach[cnt][w] |= add;
                }
            }
        }

        int validBits = (limit + 1) % 64;
        if (validBits != 0) {
            uint64_t mask = (1ULL << validBits) - 1;
            for (auto& row : reach) row.back() &= mask;
        }

        return reach;
    };

    // dp[s] is the maximum number of hidden cells so far, where the
    // current contestant's hidden cells have actual-score sum exactly s.
    int firstCap = cap[0];
    vector<int> dp(firstCap + 1, NEG);
    dp[0] = 0;

    for (int value : score[order[0]]) {
        for (int s = firstCap; s >= value; --s) {
            if (dp[s - value] != NEG) {
                dp[s] = max(dp[s], dp[s - value] + 1);
            }
        }
    }

    for (int pos = 1; pos + 1 < N; ++pos) {
        int leftCap = cap[pos - 1];
        int rightCap = cap[pos];

        vector<int> pref(leftCap + 1);
        pref[0] = dp[0];
        for (int s = 1; s <= leftCap; ++s) {
            pref[s] = max(pref[s - 1], dp[s]);
        }

        vector<int> ndp(rightCap + 1, NEG);
        const vector<int>& values = score[order[pos]];

        if (rightCap == 0) {
            int zeros = count(values.begin(), values.end(), 0);

            for (int cnt = 0; cnt <= zeros; ++cnt) {
                int upperUncertainty = K * cnt;
                if (upperUncertainty > leftCap) break;

                int previous = pref[leftCap - upperUncertainty];
                if (previous != NEG) {
                    ndp[0] = max(ndp[0], previous + cnt);
                }
            }
        } else if (leftCap == 0) {
            int fullScores = count(values.begin(), values.end(), K);

            for (int cnt = 0; cnt <= fullScores; ++cnt) {
                int hiddenSum = K * cnt;
                if (hiddenSum > rightCap) break;

                if (dp[0] != NEG) {
                    ndp[hiddenSum] = max(ndp[hiddenSum], dp[0] + cnt);
                }
            }
        } else {
            auto reach = subsetReachability(values, rightCap);

            for (int cnt = 0; cnt <= M; ++cnt) {
                for (int hiddenSum = 0;
                     hiddenSum <= rightCap;
                     ++hiddenSum) {
                    if (((reach[cnt][hiddenSum / 64]
                          >> (hiddenSum % 64)) & 1ULL) == 0) {
                        continue;
                    }

                    int upperUncertainty = K * cnt - hiddenSum;
                    if (upperUncertainty > leftCap) continue;

                    int previous = pref[leftCap - upperUncertainty];
                    if (previous != NEG) {
                        ndp[hiddenSum] =
                            max(ndp[hiddenSum], previous + cnt);
                    }
                }
            }
        }

        dp.swap(ndp);
    }

    int lastCap = cap[N - 2];
    vector<int> pref(lastCap + 1);
    pref[0] = dp[0];
    for (int s = 1; s <= lastCap; ++s) {
        pref[s] = max(pref[s - 1], dp[s]);
    }

    // For the final contestant, only upward uncertainty matters.
    vector<int> bestByDeficit(lastCap + 1, NEG);
    bestByDeficit[0] = 0;

    for (int value : score[order[N - 1]]) {
        int deficit = K - value;
        for (int sum = lastCap; sum >= deficit; --sum) {
            if (bestByDeficit[sum - deficit] != NEG) {
                bestByDeficit[sum] =
                    max(bestByDeficit[sum],
                        bestByDeficit[sum - deficit] + 1);
            }
        }
    }

    int maximumHidden = 0;
    for (int deficit = 0; deficit <= lastCap; ++deficit) {
        if (bestByDeficit[deficit] == NEG) continue;

        int previous = pref[lastCap - deficit];
        if (previous != NEG) {
            maximumHidden =
                max(maximumHidden, previous + bestByDeficit[deficit]);
        }
    }

    cout << N * M - maximumHidden << '\n';
    return 0;
}