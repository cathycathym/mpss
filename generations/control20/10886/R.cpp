#include <iostream>
#include <vector>
#include <numeric>
#include <unordered_map>
#include <limits>
#include <algorithm>

using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<int> a(n);
    for (int &x : a) cin >> x;

    int all_gcd = 0;
    for (int x : a) all_gcd = gcd(all_gcd, x);

    if (all_gcd == 1) {
        cout << 0 << '\n';
        return 0;
    }

    vector<int> prefix(n + 1, 0), suffix(n + 1, 0);

    for (int i = 0; i < n; ++i)
        prefix[i + 1] = gcd(prefix[i], a[i]);

    for (int i = n - 1; i >= 0; --i)
        suffix[i] = gcd(suffix[i + 1], a[i]);

    for (int i = 0; i < n; ++i) {
        int other_gcd = gcd(prefix[i], suffix[i + 1]);
        if (gcd(a[i] + 1, other_gcd) == 1) {
            cout << 1 << '\n';
            return 0;
        }
    }

    /*
       If adding one cookie never works, then n <= 9.

       For each i, choose a prime p_i dividing both a_i + 1 and the
       GCD of all other numbers. Such primes are distinct, and a_i is
       divisible by every p_j with j != i. Thus a_i is divisible by a
       product of n-1 distinct primes. The product of the smallest nine
       distinct primes exceeds 10^7, so n-1 <= 8.
    */

    int best = numeric_limits<int>::max();

    for (int i = 0; i < n; ++i) {
        int other_gcd = gcd(prefix[i], suffix[i + 1]);

        for (int add = 1; add < best; ++add) {
            if (gcd(a[i] + add, other_gcd) == 1) {
                best = add;
                break;
            }
        }
    }

    /*
       dp[g] is the minimum total number of added cookies after processing
       the current prefix such that its GCD is g. For an equal GCD, only
       the cheapest state matters.
    */
    unordered_map<int, int> dp;
    dp.reserve(1024);
    dp[0] = 0;

    for (int value : a) {
        unordered_map<int, int> next;
        next.reserve(dp.size() * 4 + 16);

        for (const auto &[current_gcd, cost] : dp) {
            if (cost >= best) continue;

            for (int add = 0; cost + add < best; ++add) {
                int new_cost = cost + add;
                int new_gcd = gcd(current_gcd, value + add);

                if (new_gcd == 1) {
                    best = new_cost;
                    break;
                }

                auto it = next.find(new_gcd);
                if (it == next.end() || new_cost < it->second)
                    next[new_gcd] = new_cost;
            }
        }

        dp.swap(next);
    }

    cout << best << '\n';
    return 0;
}
