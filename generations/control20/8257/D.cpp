#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
const int64 INF = (int64)4e18;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, L;
    cin >> N >> L;

    vector<int> balls(N);
    for (int &v : balls) cin >> v;
    sort(balls.begin(), balls.end());

    vector<int> pos, cnt;
    for (int v : balls) {
        if (pos.empty() || pos.back() != v) {
            pos.push_back(v);
            cnt.push_back(1);
        } else {
            ++cnt.back();
        }
    }

    int M = (int)pos.size();

    if (M > 1414) {
        int Q;
        cin >> Q;
        while (Q--) {
            int S, G, T;
            cin >> S >> G >> T;
            cout << "No\n";
        }
        return 0;
    }

    vector<int64> pref(M + 1, 0);
    for (int i = 0; i < M; ++i) {
        pref[i + 1] = pref[i] + cnt[i];
    }

    vector<int64> finishLeft(M), finishRight(M);

    if (M == 1) {
        finishLeft[0] = finishRight[0] = 0;
    } else {
        vector<int64> leftToLeft(1, 0), rightToLeft(1, INF);
        vector<int64> leftToRight(1, INF), rightToRight(1, 0);

        for (int len = M - 1; len >= 1; --len) {
            int states = M - len + 1;

            vector<int64> nextLeftToLeft(states, INF);
            vector<int64> nextRightToLeft(states, INF);
            vector<int64> nextLeftToRight(states, INF);
            vector<int64> nextRightToRight(states, INF);

            for (int i = 0; i < states; ++i) {
                int j = i + len - 1;
                int64 visited = pref[j + 1] - pref[i];
                int64 weight = (int64)N - visited + 1;

                if (i > 0) {
                    int64 fromLeft = pos[i] - pos[i - 1];
                    int64 fromRight = pos[j] - pos[i - 1];

                    if (leftToLeft[i - 1] < INF / 2) {
                        nextLeftToLeft[i] = min(
                            nextLeftToLeft[i],
                            weight * fromLeft + leftToLeft[i - 1]
                        );
                        nextRightToLeft[i] = min(
                            nextRightToLeft[i],
                            weight * fromRight + leftToLeft[i - 1]
                        );
                    }

                    if (leftToRight[i - 1] < INF / 2) {
                        nextLeftToRight[i] = min(
                            nextLeftToRight[i],
                            weight * fromLeft + leftToRight[i - 1]
                        );
                        nextRightToRight[i] = min(
                            nextRightToRight[i],
                            weight * fromRight + leftToRight[i - 1]
                        );
                    }
                }

                if (j + 1 < M) {
                    int64 fromLeft = pos[j + 1] - pos[i];
                    int64 fromRight = pos[j + 1] - pos[j];

                    if (rightToLeft[i] < INF / 2) {
                        nextLeftToLeft[i] = min(
                            nextLeftToLeft[i],
                            weight * fromLeft + rightToLeft[i]
                        );
                        nextRightToLeft[i] = min(
                            nextRightToLeft[i],
                            weight * fromRight + rightToLeft[i]
                        );
                    }

                    if (rightToRight[i] < INF / 2) {
                        nextLeftToRight[i] = min(
                            nextLeftToRight[i],
                            weight * fromLeft + rightToRight[i]
                        );
                        nextRightToRight[i] = min(
                            nextRightToRight[i],
                            weight * fromRight + rightToRight[i]
                        );
                    }
                }
            }

            leftToLeft.swap(nextLeftToLeft);
            rightToLeft.swap(nextRightToLeft);
            leftToRight.swap(nextLeftToRight);
            rightToRight.swap(nextRightToRight);
        }

        for (int i = 0; i < M; ++i) {
            finishLeft[i] = min(leftToLeft[i], rightToLeft[i]);
            finishRight[i] = min(leftToRight[i], rightToRight[i]);
        }
    }

    int Q;
    cin >> Q;

    while (Q--) {
        int S, G;
        int64 T;
        cin >> S >> G >> T;

        int p = lower_bound(pos.begin(), pos.end(), G) - pos.begin();

        int64 toLeftEnd = INF;
        int64 toRightEnd = INF;

        if (p < M && pos[p] == G) {
            toLeftEnd = finishLeft[p];
            toRightEnd = finishRight[p];
        } else {
            int64 initialWeight = (int64)N + 1;

            if (p > 0) {
                int i = p - 1;
                int64 firstCost = initialWeight * (G - pos[i]);
                toLeftEnd = min(toLeftEnd, firstCost + finishLeft[i]);
                toRightEnd = min(toRightEnd, firstCost + finishRight[i]);
            }

            if (p < M) {
                int i = p;
                int64 firstCost = initialWeight * (pos[i] - G);
                toLeftEnd = min(toLeftEnd, firstCost + finishLeft[i]);
                toRightEnd = min(toRightEnd, firstCost + finishRight[i]);
            }
        }

        int64 movementCost = min(
            toLeftEnd + llabs((int64)pos.front() - S),
            toRightEnd + llabs((int64)pos.back() - S)
        );

        int64 totalTime = movementCost + N;
        cout << (totalTime <= T ? "Yes\n" : "No\n");
    }

    return 0;
}
