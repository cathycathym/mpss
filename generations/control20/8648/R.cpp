#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
using i128 = __int128_t;

const int64 INF = (1LL << 62);
const int64 DIRECT_LIMIT = 1000000;

struct Construction {
    int64 l, r;
};

struct ReachableInterval {
    int64 l, r;
    int64 costAtL; // Cost of one valid path to l.
};

static int64 addCost(int64 x, int64 y) {
    i128 z = (i128)x + y;
    return z >= INF ? INF : (int64)z;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, Q;
    cin >> N >> Q;

    int64 D, A, B;
    cin >> D >> A >> B;

    vector<Construction> construction(N);
    for (auto &[l, r] : construction) {
        cin >> l >> r;
    }

    vector<int64> query(Q);
    int64 maximumX = 0;
    for (int i = 0; i < Q; ++i) {
        cin >> query[i];
        maximumX = max(maximumX, query[i]);
    }

    /*
     * Exact dynamic programming for subtask 1.
     *
     * dp[x] = minimum cost to land on x.
     * A forbidden step has infinite distance.
     */
    if (maximumX <= DIRECT_LIMIT) {
        int M = (int)maximumX;

        vector<int> difference(M + 2, 0);
        for (const auto &[l64, r64] : construction) {
            if (l64 > M) continue;

            int l = (int)l64;
            int r = (int)min<int64>(r64, M);
            ++difference[l];
            --difference[r + 1];
        }

        vector<char> forbidden(M + 1, false);
        int active = 0;
        for (int x = 1; x <= M; ++x) {
            active += difference[x];
            forbidden[x] = (active != 0);
        }

        vector<int64> dp(M + 1, INF);
        dp[0] = 0;

        for (int x = 1; x <= M; ++x) {
            if (forbidden[x]) continue;

            if (dp[x - 1] != INF) {
                dp[x] = min(dp[x], addCost(dp[x - 1], A));
            }

            if ((int64)x >= D && dp[x - (int)D] != INF) {
                dp[x] = min(dp[x], addCost(dp[x - (int)D], B));
            }
        }

        for (int64 x : query) {
            if (dp[(int)x] == INF) cout << -1 << '\n';
            else cout << dp[(int)x] << '\n';
        }
        return 0;
    }

    /*
     * Reachability on large coordinates.
     *
     * In every allowed connected interval [a,b], if t is a reachable
     * position satisfying
     *
     *     a-D <= t <= b-D,
     *
     * then a jump reaches t+D, and every allowed step from t+D through b
     * is reachable. Consequently, the reachable part of each allowed
     * interval is a suffix.
     */
    vector<pair<int64, int64>> allowed;
    int64 left = 0;

    for (const auto &[l, r] : construction) {
        if (left <= maximumX && left <= l - 1) {
            allowed.push_back({left, min(maximumX, l - 1)});
        }

        if (r == numeric_limits<int64>::max()) {
            left = r;
        } else {
            left = r + 1;
        }

        if (left > maximumX) break;
    }

    if (left <= maximumX) {
        allowed.push_back({left, maximumX});
    }

    vector<ReachableInterval> reachable;

    for (auto [a, b] : allowed) {
        if (a > b) continue;

        if (a == 0) {
            reachable.push_back({0, b, 0});
            continue;
        }

        int64 lower = max<int64>(0, a - D);
        if (b < D) continue;
        int64 upper = b - D;
        if (lower > upper) continue;

        int lo = 0, hi = (int)reachable.size();
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            if (reachable[mid].r < lower) lo = mid + 1;
            else hi = mid;
        }

        if (lo == (int)reachable.size()) continue;

        const ReachableInterval &source = reachable[lo];
        int64 takeoff = max(lower, source.l);

        if (takeoff > source.r || takeoff > upper) continue;

        int64 landing = takeoff + D;
        int64 sourceCost =
            addCost(source.costAtL, (takeoff - source.l) * A);
        int64 landingCost = addCost(sourceCost, B);

        reachable.push_back({landing, b, landingCost});
    }

    auto findReachableInterval = [&](int64 x) -> int {
        int lo = 0, hi = (int)reachable.size();

        while (lo < hi) {
            int mid = (lo + hi) / 2;
            if (reachable[mid].l <= x) lo = mid + 1;
            else hi = mid;
        }

        int index = lo - 1;
        if (index >= 0 && x <= reachable[index].r) return index;
        return -1;
    };

    for (int64 x : query) {
        int index = findReachableInterval(x);

        if (index == -1) {
            cout << -1 << '\n';
            continue;
        }

        /*
         * If B = D*A, replacing D single steps by one jump never changes
         * the cost. Thus every path to x costs exactly A*x, making this
         * branch an exact solution to subtask 3.
         */
        if ((i128)B == (i128)D * A) {
            cout << (int64)((i128)x * A) << '\n';
        } else {
            const ReachableInterval &segment = reachable[index];
            int64 answer =
                addCost(segment.costAtL, (x - segment.l) * A);
            cout << answer << '\n';
        }
    }

    return 0;
}
