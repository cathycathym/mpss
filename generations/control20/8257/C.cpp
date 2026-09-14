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

    vector<int> x;
    vector<int> cnt;
    for (int v : balls) {
        if (x.empty() || x.back() != v) {
            x.push_back(v);
            cnt.push_back(1);
        } else {
            ++cnt.back();
        }
    }

    int M = (int)x.size();
    int Q;

    // More than 1414 distinct integer positions already require
    // over 500000 seconds of movement contribution alone.
    if (M > 1414) {
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

    vector<int64> costEndLeft(M), costEndRight(M);

    if (M == 1) {
        costEndLeft[0] = costEndRight[0] = 0;
    } else {
        // For intervals of the current length:
        // aL/aR force the route to finish at the global leftmost position.
        // bL/bR force it to finish at the global rightmost position.
        vector<int64> aL(1, 0), aR(1, INF);
        vector<int64> bL(1, INF), bR(1, 0);

        for (int len = M - 1; len >= 1; --len) {
            int states = M - len + 1;
            vector<int64> naL(states, INF), naR(states, INF);
            vector<int64> nbL(states, INF), nbR(states, INF);

            for (int i = 0; i < states; ++i) {
                int j = i + len - 1;
                int64 visited = pref[j + 1] - pref[i];
                int64 weight = (int64)N - visited + 1;

                if (i > 0) {
                    int64 dFromLeft = x[i] - x[i - 1];
                    int64 dFromRight = x[j] - x[i - 1];

                    if (aL[i - 1] < INF / 2) {
                        naL[i] = min(naL[i],
                                     weight * dFromLeft + aL[i - 1]);
                        naR[i] = min(naR[i],
                                     weight * dFromRight + aL[i - 1]);
                    }
                    if (bL[i - 1] < INF / 2) {
                        nbL[i] = min(nbL[i],
                                     weight * dFromLeft + bL[i - 1]);
                        nbR[i] = min(nbR[i],
                                     weight * dFromRight + bL[i - 1]);
                    }
                }

                if (j + 1 < M) {
                    int64 dFromLeft = x[j + 1] - x[i];
                    int64 dFromRight = x[j + 1] - x[j];

                    if (aR[i] < INF / 2) {
                        naL[i] = min(naL[i],
                                     weight * dFromLeft + aR[i]);
                        naR[i] = min(naR[i],
                                     weight * dFromRight + aR[i]);
                    }
                    if (bR[i] < INF / 2) {
                        nbL[i] = min(nbL[i],
                                     weight * dFromLeft + bR[i]);
                        nbR[i] = min(nbR[i],
                                     weight * dFromRight + bR[i]);
                    }
                }
            }

            aL.swap(naL);
            aR.swap(naR);
            bL.swap(nbL);
            bR.swap(nbR);
        }

        for (int i = 0; i < M; ++i) {
            costEndLeft[i] = min(aL[i], aR[i]);
            costEndRight[i] = min(bL[i], bR[i]);
        }
    }

    cin >> Q;
    while (Q--) {
        int S, G, T;
        cin >> S >> G >> T;

        int p = lower_bound(x.begin(), x.end(), G) - x.begin();
        int64 startWeight = (int64)N + 1;
        int64 bestLeft = INF;
        int64 bestRight = INF;

        if (p < M && x[p] == G) {
            bestLeft = costEndLeft[p];
            bestRight = costEndRight[p];
        } else {
            if (p > 0) {
                int i = p - 1;
                int64 initial = startWeight * (G - x[i]);
                bestLeft = min(bestLeft, initial + costEndLeft[i]);
                bestRight = min(bestRight, initial + costEndRight[i]);
            }
            if (p < M) {
                int i = p;
                int64 initial = startWeight * (x[i] - G);
                bestLeft = min(bestLeft, initial + costEndLeft[i]);
                bestRight = min(bestRight, initial + costEndRight[i]);
            }
        }

        int64 movementCost = min(
            bestLeft + llabs((int64)x.front() - S),
            bestRight + llabs((int64)x.back() - S)
        );

        int64 totalCost = movementCost + N;
        cout << (totalCost <= T ? "Yes\n" : "No\n");
    }

    return 0;
}
