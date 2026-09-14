#include <bits/stdc++.h>
using namespace std;

constexpr int MOD = 1'000'000'007;

struct Event {
    int step;
    int delta;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    cin >> N;

    vector<long long> d(N + 1), x(N + 1);
    for (int i = 1; i <= N; ++i) {
        cin >> d[i] >> x[i];
    }

    const int B = static_cast<int>(sqrt(N)) + 1;

    vector<vector<Event>> events(N + 2);
    vector<vector<int>> active(B + 1);
    for (int step = 1; step <= B; ++step) {
        active[step].assign(step, 0);
    }

    vector<int> dp(N + 1, 0);
    dp[1] = 1;

    int answer = 0;

    for (int city = 1; city <= N; ++city) {
        for (const Event& event : events[city]) {
            int residue = city % event.step;
            int& value = active[event.step][residue];
            value += event.delta;
            if (value >= MOD) value -= MOD;
        }

        for (int step = 1; step <= B; ++step) {
            dp[city] += active[step][city % step];
            if (dp[city] >= MOD) dp[city] -= MOD;
        }

        answer += dp[city];
        if (answer >= MOD) answer -= MOD;

        if (d[city] == 0 || x[city] == 0 || d[city] > N - city) {
            continue;
        }

        long long stops = min(x[city], (N - city) / d[city]);

        if (d[city] <= B) {
            int step = static_cast<int>(d[city]);
            int firstStop = city + step;
            long long afterLast = city + (stops + 1) * d[city];

            events[firstStop].push_back({step, dp[city]});

            if (afterLast <= N) {
                int removal = (dp[city] == 0 ? 0 : MOD - dp[city]);
                events[static_cast<int>(afterLast)].push_back({step, removal});
            }
        } else {
            for (long long t = 1; t <= stops; ++t) {
                int nextCity = static_cast<int>(city + t * d[city]);
                dp[nextCity] += dp[city];
                if (dp[nextCity] >= MOD) dp[nextCity] -= MOD;
            }
        }
    }

    cout << answer << '\n';
    return 0;
}