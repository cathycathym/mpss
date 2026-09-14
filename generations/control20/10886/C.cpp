#include <algorithm>
#include <cstdint>
#include <functional>
#include <iostream>
#include <numeric>
#include <unordered_set>
#include <vector>

using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<long long> a(n);
    long long allGcd = 0;

    for (long long &v : a) {
        cin >> v;
        allGcd = std::gcd(allGcd, v);
    }

    if (allGcd == 1) {
        cout << 0 << '\n';
        return 0;
    }

    // For n >= 10, the proof above guarantees that one increment suffices.
    if (n >= 10) {
        cout << 1 << '\n';
        return 0;
    }

    // Obtain an upper bound by changing only the first box.
    long long gcdOfOthers = 0;
    for (int i = 1; i < n; ++i) {
        gcdOfOthers = std::gcd(gcdOfOthers, a[i]);
    }

    int upperBound = 1;
    while (std::gcd(a[0] + upperBound, gcdOfOthers) != 1) {
        ++upperBound;
    }

    for (int total = 1; total <= upperBound; ++total) {
        unordered_set<uint64_t> failed;
        failed.reserve(4096);

        function<bool(int, int, long long)> possible =
            [&](int index, int remaining, long long currentGcd) -> bool {
                if (currentGcd == 1) {
                    return true;
                }

                if (index == n - 1) {
                    long long value = a[index] + remaining;
                    return std::gcd(currentGcd, value) == 1;
                }

                uint64_t key =
                    (static_cast<uint64_t>(index) << 56) |
                    (static_cast<uint64_t>(remaining) << 40) |
                    static_cast<uint64_t>(currentGcd);

                if (failed.find(key) != failed.end()) {
                    return false;
                }

                for (int add = 0; add <= remaining; ++add) {
                    long long value = a[index] + add;
                    long long nextGcd =
                        (currentGcd == 0 ? value
                                         : std::gcd(currentGcd, value));

                    if (possible(index + 1, remaining - add, nextGcd)) {
                        return true;
                    }
                }

                failed.insert(key);
                return false;
            };

        if (possible(0, total, 0)) {
            cout << total << '\n';
            return 0;
        }
    }

    return 0;
}
