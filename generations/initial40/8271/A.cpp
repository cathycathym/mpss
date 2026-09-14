#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    vector<long long> w(n + 1);
    for (int i = 1; i <= n; ++i) cin >> w[i];

    /*
      For small n, enumerate every possible set of distinct values.
      Repeated values are never useful: one occurrence can be reused as
      either factor arbitrarily many times.

      For larger n, construct valid (though not necessarily optimal) sequences
      with dynamic programming by concatenating constructions for factors.
    */
    if (n <= 22) {
        int total = 1 << n;
        vector<long long> cost(total, 0);
        vector<char> good(total, false);
        vector<long long> ans(n + 1, (1LL << 62));

        vector<vector<pair<int, int>>> factors(n + 1);
        for (int x = 2; x <= n; ++x) {
            for (int d = 2; d * d <= x; ++d) {
                if (x % d == 0)
                    factors[x].push_back({d, x / d});
            }
        }

        for (int mask = 1; mask < total; ++mask) {
            int bit = __builtin_ctz(mask);
            cost[mask] = cost[mask ^ (1 << bit)] + w[bit + 1];

            if (!(mask & 1)) continue;

            bool ok = true;
            for (int x = 2; x <= n && ok; ++x) {
                if (!(mask & (1 << (x - 1)))) continue;

                if (mask & (1 << (x - 2))) continue;

                bool madeByProduct = false;
                for (auto p : factors[x]) {
                    if ((mask & (1 << (p.first - 1))) &&
                        (mask & (1 << (p.second - 1)))) {
                        madeByProduct = true;
                        break;
                    }
                }
                if (!madeByProduct) ok = false;
            }

            if (!ok) continue;
            good[mask] = true;

            for (int x = 1; x <= n; ++x) {
                if (mask & (1 << (x - 1)))
                    ans[x] = min(ans[x], cost[mask]);
            }
        }

        for (int i = 1; i <= n; ++i)
            cout << ans[i] << '\n';
        return 0;
    }

    const long long INF = (1LL << 62);
    vector<long long> dp(n + 1, INF);
    dp[1] = w[1];

    for (int x = 2; x <= n; ++x) {
        dp[x] = dp[x - 1] + w[x];

        for (int d = 2; d * d <= x; ++d) {
            if (x % d != 0) continue;
            int e = x / d;
            dp[x] = min(dp[x], dp[d] + dp[e] + w[x]);
        }
    }

    for (int i = 1; i <= n; ++i)
        cout << dp[i] << '\n';

    return 0;
}
