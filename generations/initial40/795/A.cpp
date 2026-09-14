#include <algorithm>
#include <iostream>
#include <limits>
#include <vector>

using namespace std;

struct Event {
    long long frequency;
    int cores;
    long long value;
    bool computer;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<Event> events;
    events.reserve(n + 2000);

    int totalCores = 0;

    for (int i = 0; i < n; ++i) {
        int c;
        long long f, v;
        cin >> c >> f >> v;
        events.push_back({f, c, v, true});
        totalCores += c;
    }

    int m;
    cin >> m;

    for (int i = 0; i < m; ++i) {
        int c;
        long long f, v;
        cin >> c >> f >> v;
        events.push_back({f, c, v, false});
    }

    sort(events.begin(), events.end(), [](const Event& a, const Event& b) {
        if (a.frequency != b.frequency)
            return a.frequency > b.frequency;

        // At equal frequency, these computers can serve these orders.
        return a.computer > b.computer;
    });

    const long long NEG = numeric_limits<long long>::min() / 4;
    vector<long long> dp(totalCores + 1, NEG);
    dp[0] = 0;

    int availableLimit = 0;

    for (const Event& event : events) {
        if (event.computer) {
            // Descending iteration makes this a 0/1 transition.
            for (int k = availableLimit; k >= 0; --k) {
                if (dp[k] == NEG)
                    continue;

                dp[k + event.cores] =
                    max(dp[k + event.cores], dp[k] - event.value);
            }
            availableLimit += event.cores;
        } else {
            // Ascending iteration prevents accepting this order repeatedly.
            for (int k = event.cores; k <= availableLimit; ++k) {
                if (dp[k] == NEG)
                    continue;

                dp[k - event.cores] =
                    max(dp[k - event.cores], dp[k] + event.value);
            }
        }
    }

    long long answer = 0;
    for (long long profit : dp)
        answer = max(answer, profit);

    cout << answer << '\n';
    return 0;
}