#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>

using namespace std;

int gcdInt(int a, int b) {
    while (b != 0) {
        int r = a % b;
        a = b;
        b = r;
    }
    return a;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<int> a(n);
    int totalGcd = 0;

    for (int &x : a) {
        cin >> x;
        totalGcd = gcdInt(totalGcd, x);
    }

    if (totalGcd == 1) {
        cout << 0 << '\n';
        return 0;
    }

    /*
    If adding one cookie to every individual box fails, then for each box i
    there is a prime p_i dividing all other boxes and a_i + 1. These primes
    are pairwise distinct, and every a_j is divisible by all p_i with i != j.

    For n >= 10 this would make a_j divisible by at least nine distinct
    primes, whose smallest possible product is
    2*3*5*7*11*13*17*19*23 > 10^7, a contradiction.
    */
    if (n >= 10) {
        cout << 1 << '\n';
        return 0;
    }

    // A valid upper bound: change only the first box.
    int gcdOfOthers = 0;
    for (int i = 1; i < n; ++i) {
        gcdOfOthers = gcdInt(gcdOfOthers, a[i]);
    }

    int best = 1;
    while (gcdInt(a[0] + best, gcdOfOthers) != 1) {
        ++best;
    }

    // For each attainable prefix GCD, store the least total addition.
    unordered_map<int, int> dp;
    dp.reserve(1024);
    dp[0] = 0;

    for (int value : a) {
        unordered_map<int, int> next;
        next.reserve(dp.size() * static_cast<size_t>(best + 1));

        for (const auto &state : dp) {
            int currentGcd = state.first;
            int currentCost = state.second;

            if (currentCost >= best)
                continue;

            int maxAdd = best - currentCost - 1;

            for (int add = 0; add <= maxAdd; ++add) {
                int newCost = currentCost + add;
                int newGcd = gcdInt(currentGcd, value + add);

                if (newGcd == 1) {
                    best = min(best, newCost);
                    continue;
                }

                if (newCost >= best)
                    continue;

                auto it = next.find(newGcd);
                if (it == next.end() || newCost < it->second) {
                    next[newGcd] = newCost;
                }
            }
        }

        dp.swap(next);
    }

    cout << best << '\n';
    return 0;
}
