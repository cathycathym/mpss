#include <iostream>
#include <vector>
#include <unordered_map>
#include <numeric>
#include <algorithm>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<int> a(n);
    int allGcd = 0;
    for (int &v : a) {
        cin >> v;
        allGcd = gcd(allGcd, v);
    }

    if (allGcd == 1) {
        cout << 0 << '\n';
        return 0;
    }

    // A valid upper bound: leave a[0] unchanged and increase a[1]
    // until the two numbers are coprime.
    int best = 1;
    while (gcd(a[0], a[1] + best) != 1) {
        ++best;
    }

    // Try several other ordered pairs to reduce the upper bound.
    int limit = min(n, 64);
    for (int i = 0; i < limit; ++i) {
        for (int j = 0; j < limit; ++j) {
            if (i == j) continue;
            for (int add = 1; add < best; ++add) {
                if (gcd(a[i], a[j] + add) == 1) {
                    best = add;
                    break;
                }
            }
            if (best == 1) {
                cout << 1 << '\n';
                return 0;
            }
        }
    }

    // gcd -> minimum total cost for the processed prefix.
    // GCD 0 is the neutral starting state.
    unordered_map<int, int> dp;
    dp.reserve(1024);
    dp[0] = 0;

    for (int value : a) {
        unordered_map<int, int> next;
        next.reserve(dp.size() * 3 + 16);

        for (const auto &state : dp) {
            int currentGcd = state.first;
            int currentCost = state.second;

            if (currentCost >= best) continue;

            int maxAdd = best - currentCost - 1;
            for (int add = 0; add <= maxAdd; ++add) {
                int newCost = currentCost + add;
                int newGcd = gcd(currentGcd, value + add);

                if (newGcd == 1) {
                    best = min(best, newCost);
                    continue;
                }

                if (newCost >= best) continue;

                auto it = next.find(newGcd);
                if (it == next.end() || newCost < it->second) {
                    next[newGcd] = newCost;
                }
            }
        }

        if (best == 1) {
            cout << 1 << '\n';
            return 0;
        }

        for (auto it = next.begin(); it != next.end(); ) {
            if (it->second >= best) {
                it = next.erase(it);
            } else {
                ++it;
            }
        }

        dp.swap(next);
    }

    cout << best << '\n';
    return 0;
}
