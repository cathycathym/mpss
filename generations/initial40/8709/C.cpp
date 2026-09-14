#include <bits/stdc++.h>
using namespace std;

static const long long MOD = 1000000007LL;

struct Event {
    int d;
    long long delta;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    cin >> N;

    vector<long long> d(N + 1), x(N + 1);
    for (int i = 1; i <= N; ++i) cin >> d[i] >> x[i];

    const int B = 320;

    vector<vector<Event>> events(N + 2);
    vector<vector<long long>> active(B + 1);
    for (int step = 1; step <= B; ++step) {
        active[step].assign(step, 0);
    }

    vector<long long> dp(N + 1, 0);
    dp[1] = 1;

    long long answer = 0;

    for (int i = 1; i <= N; ++i) {
        // Apply scheduled additions/removals before computing dp[i].
        for (const Event &e : events[i]) {
            int residue = i % e.d;
            active[e.d][residue] += e.delta;
            active[e.d][residue] %= MOD;
            if (active[e.d][residue] < 0) active[e.d][residue] += MOD;
        }

        // Contributions from all currently active small-step routes.
        for (int step = 1; step <= B; ++step) {
            dp[i] += active[step][i % step];
            if (dp[i] >= MOD) dp[i] -= MOD;
        }

        answer += dp[i];
        answer %= MOD;

        if (d[i] == 0 || x[i] == 0) continue;

        long long maxStops = (N - i) / d[i];
        long long stops = min(x[i], maxStops);
        if (stops == 0) continue;

        if (d[i] <= B) {
            int step = (int)d[i];
            long long first = i + d[i];
            long long afterLast = i + (stops + 1) * d[i];

            events[(int)first].push_back({step, dp[i]});
            if (afterLast <= N) {
                events[(int)afterLast].push_back({step, -dp[i]});
            }
        } else {
            for (long long t = 1; t <= stops; ++t) {
                int city = (int)(i + t * d[i]);
                dp[city] += dp[i];
                if (dp[city] >= MOD) dp[city] -= MOD;
            }
        }
    }

    cout << answer << '\n';
    return 0;
}
