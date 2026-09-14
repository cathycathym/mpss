#include <algorithm>
#include <iostream>
#include <limits>
#include <vector>

using namespace std;

struct Event {
    long long frequency;
    int cores;
    long long value;
    bool isComputer;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<Event> events;
    events.reserve(n + 2000);

    int totalSupply = 0;
    for (int i = 0; i < n; ++i) {
        int cores;
        long long frequency, price;
        cin >> cores >> frequency >> price;
        events.push_back({frequency, cores, -price, true});
        totalSupply += cores;
    }

    int m;
    cin >> m;

    int remainingDemand = 0;
    for (int i = 0; i < m; ++i) {
        int cores;
        long long frequency, payment;
        cin >> cores >> frequency >> payment;
        events.push_back({frequency, cores, payment, false});
        remainingDemand += cores;
    }

    sort(events.begin(), events.end(),
         [](const Event& a, const Event& b) {
             if (a.frequency != b.frequency)
                 return a.frequency > b.frequency;
             return a.isComputer > b.isComputer;
         });

    const int limit = min(totalSupply, remainingDemand);
    const long long NEG = numeric_limits<long long>::lowest() / 4;

    vector<long long> dp(limit + 1, NEG);
    dp[0] = 0;

    int currentMaximum = 0;

    for (const Event& event : events) {
        int currentLimit = min(limit, remainingDemand);

        if (event.isComputer) {
            for (int available = min(currentMaximum, currentLimit - 1);
                 available >= 0; --available) {
                if (dp[available] == NEG)
                    continue;

                int nextAvailable =
                    min(currentLimit, available + event.cores);

                dp[nextAvailable] =
                    max(dp[nextAvailable],
                        dp[available] + event.value);
            }

            currentMaximum =
                min(currentLimit, currentMaximum + event.cores);
        } else {
            for (int available = event.cores;
                 available <= currentMaximum; ++available) {
                if (dp[available] == NEG)
                    continue;

                dp[available - event.cores] =
                    max(dp[available - event.cores],
                        dp[available] + event.value);
            }

            remainingDemand -= event.cores;
            int newLimit = min(limit, remainingDemand);

            if (currentMaximum > newLimit) {
                long long best = dp[newLimit];
                for (int available = newLimit + 1;
                     available <= currentMaximum; ++available) {
                    best = max(best, dp[available]);
                    dp[available] = NEG;
                }
                dp[newLimit] = best;
                currentMaximum = newLimit;
            }
        }
    }

    cout << *max_element(dp.begin(), dp.end()) << '\n';
    return 0;
}