#include <bits/stdc++.h>
using namespace std;

static const int MOD = 1000000007;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    cin >> N;

    vector<long long> d(N + 1), x(N + 1);
    for (int i = 1; i <= N; ++i) cin >> d[i] >> x[i];

    int B = (int)sqrt(N) + 1;

    // Events change active contributions of short-step routes at a city.
    vector<vector<pair<int, int>>> events(N + 1);

    // active[step][remainder] is the total contribution currently active
    // for cities with this remainder modulo step.
    vector<vector<int>> active(B + 1);
    for (int step = 1; step <= B; ++step) {
        active[step].assign(step, 0);
    }

    vector<int> dp(N + 1, 0);
    dp[1] = 1;

    for (int city = 1; city <= N; ++city) {
        for (const auto &event : events[city]) {
            int step = event.first;
            int value = event.second;
            int rem = city % step;

            active[step][rem] += value;
            if (active[step][rem] >= MOD) active[step][rem] -= MOD;
        }

        long long incoming = 0;
        for (int step = 1; step <= B; ++step) {
            incoming += active[step][city % step];
        }
        dp[city] += (int)(incoming % MOD);
        if (dp[city] >= MOD) dp[city] -= MOD;

        if (d[city] == 0 || x[city] == 0 || d[city] > N - city) continue;

        long long stops = min(x[city], (long long)(N - city) / d[city]);

        if (d[city] <= B) {
            int step = (int)d[city];
            int start = city + step;
            int finish = (int)(city + (stops + 1) * d[city]);

            events[start].push_back({step, dp[city]});
            if (finish <= N) {
                events[finish].push_back({step, (dp[city] == 0 ? 0 : MOD - dp[city])});
            }
        } else {
            for (long long t = 1; t <= stops; ++t) {
                int nextCity = (int)(city + t * d[city]);
                dp[nextCity] += dp[city];
                if (dp[nextCity] >= MOD) dp[nextCity] -= MOD;
            }
        }
    }

    int answer = 0;
    for (int city = 1; city <= N; ++city) {
        answer += dp[city];
        if (answer >= MOD) answer -= MOD;
    }

    cout << answer << '\n';
    return 0;
}
