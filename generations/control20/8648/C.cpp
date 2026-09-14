#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
using i128 = __int128_t;

static const int64 INF = (int64)4e18;

struct ReachInterval {
    int64 left, right;
    int64 jumps;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, Q;
    cin >> N >> Q;

    int64 D, A, B;
    cin >> D >> A >> B;

    vector<int64> L(N), R(N);
    int64 maxCoordinate = 0;

    for (int i = 0; i < N; ++i) {
        cin >> L[i] >> R[i];
        maxCoordinate = max(maxCoordinate, R[i]);
    }

    vector<int64> X(Q);
    for (int i = 0; i < Q; ++i) {
        cin >> X[i];
        maxCoordinate = max(maxCoordinate, X[i]);
    }

    /*
     * Exact dense solution for subtask 1.
     */
    if (maxCoordinate <= 1000000) {
        int M = (int)maxCoordinate;

        vector<char> forbidden(M + 1, false);
        for (int i = 0; i < N; ++i) {
            int l = (int)L[i];
            int r = (int)R[i];
            for (int x = l; x <= r; ++x) {
                forbidden[x] = true;
            }
        }

        vector<int64> dp(M + 1, INF);
        dp[0] = 0;

        for (int x = 1; x <= M; ++x) {
            if (forbidden[x]) continue;

            if (dp[x - 1] != INF) {
                dp[x] = min(dp[x], dp[x - 1] + A);
            }

            if ((int64)x >= D && dp[(int64)x - D] != INF) {
                dp[x] = min(dp[x], dp[(int64)x - D] + B);
            }
        }

        for (int64 x : X) {
            if (dp[(int)x] == INF) cout << -1 << '\n';
            else cout << dp[(int)x] << '\n';
        }
        return 0;
    }

    int64 maxQuery = *max_element(X.begin(), X.end());

    /*
     * Store the reachable suffix of every free component.
     * Their right endpoints are strictly increasing.
     */
    vector<ReachInterval> reachable;

    auto addFreeComponent = [&](int64 s, int64 e) {
        if (s > e || s > maxQuery) return;
        e = min(e, maxQuery);

        if (s == 0) {
            reachable.push_back({0, e, 0});
            return;
        }

        int64 low = s - D;
        int64 high = e - D;
        if (high < 0 || reachable.empty()) return;
        low = max<int64>(low, 0);

        int lo = 0, hi = (int)reachable.size();
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            if (reachable[mid].right >= low) hi = mid;
            else lo = mid + 1;
        }

        if (lo == (int)reachable.size()) return;

        const ReachInterval &src = reachable[lo];
        int64 y = max(low, src.left);

        if (y > high || y > src.right) return;

        int64 firstLanding = y + D;
        if (firstLanding < s || firstLanding > e) return;

        reachable.push_back({
            firstLanding,
            e,
            src.jumps + 1
        });
    };

    int64 freeStart = 0;

    for (int i = 0; i < N && freeStart <= maxQuery; ++i) {
        if (L[i] > maxQuery) break;

        addFreeComponent(freeStart, L[i] - 1);

        if (R[i] == LLONG_MAX) {
            freeStart = LLONG_MAX;
            break;
        }
        freeStart = R[i] + 1;
    }

    if (freeStart <= maxQuery) {
        addFreeComponent(freeStart, maxQuery);
    }

    auto findReachable = [&](int64 x) -> const ReachInterval* {
        int lo = 0, hi = (int)reachable.size();

        while (lo < hi) {
            int mid = (lo + hi) / 2;
            if (reachable[mid].right >= x) hi = mid;
            else lo = mid + 1;
        }

        if (lo == (int)reachable.size()) return nullptr;
        if (reachable[lo].left <= x && x <= reachable[lo].right) {
            return &reachable[lo];
        }
        return nullptr;
    };

    i128 jumpPenalty = (i128)B - (i128)A * D;

    for (int64 x : X) {
        const ReachInterval *ri = findReachable(x);

        if (ri == nullptr) {
            cout << -1 << '\n';
            continue;
        }

        /*
         * Exact when jumpPenalty >= 0, including subtask 3.
         * For unsupported large cheap-jump instances, this is the cost of
         * a valid minimum-jump path, though it may not be globally minimum.
         */
        i128 answer = (i128)A * x + jumpPenalty * ri->jumps;

        int64 out = (int64)answer;
        cout << out << '\n';
    }

    return 0;
}
