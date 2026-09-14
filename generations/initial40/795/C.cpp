#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>

using namespace std;

struct Event {
    long long frequency;
    int cores;
    long long value;
    int type; // 0 = computer, 1 = order
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<Event> events;
    events.reserve(n + 2000);

    long long totalSupply = 0;
    for (int i = 0; i < n; ++i) {
        int c;
        long long f, v;
        cin >> c >> f >> v;
        events.push_back({f, c, -v, 0});
        totalSupply += c;
    }

    int m;
    cin >> m;

    long long totalDemand = 0;
    for (int i = 0; i < m; ++i) {
        int c;
        long long f, v;
        cin >> c >> f >> v;
        events.push_back({f, c, v, 1});
        totalDemand += c;
    }

    sort(events.begin(), events.end(), [](const Event& a, const Event& b) {
        if (a.frequency != b.frequency) {
            return a.frequency > b.frequency;
        }
        return a.type < b.type; // computers before orders at equal frequency
    });

    int limit = static_cast<int>(min(totalSupply, totalDemand));
    const long long NEG = numeric_limits<long long>::min() / 4;

    vector<long long> dp(limit + 1, NEG);
    dp[0] = 0;

    int currentMaximum = 0;

    for (const Event& event : events) {
        if (event.type == 0) {
            int upper = min(currentMaximum, limit - 1);

            for (int available = upper; available >= 0; --available) {
                if (dp[available] == NEG) {
                    continue;
                }

                int nextAvailable = min(limit, available + event.cores);
                dp[nextAvailable] = max(
                    dp[nextAvailable],
                    dp[available] + event.value
                );
            }

            currentMaximum = min(limit, currentMaximum + event.cores);
        } else {
            for (int available = event.cores;
                 available <= currentMaximum;
                 ++available) {
                if (dp[available] == NEG) {
                    continue;
                }

                dp[available - event.cores] = max(
                    dp[available - event.cores],
                    dp[available] + event.value
                );
            }
        }
    }

    cout << *max_element(dp.begin(), dp.end()) << '\n';
    return 0;
}