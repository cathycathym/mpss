#include <bits/stdc++.h>
using namespace std;

using i128 = __int128_t;
using u64 = uint64_t;

static constexpr int SHIFT = 9;
static constexpr int MASK = (1 << SHIFT) - 1;
static const i128 INF = (i128(1) << 120);

struct CustomHash {
    static uint64_t splitmix64(uint64_t x) {
        x += 0x9e3779b97f4a7c15ULL;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
        x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
        return x ^ (x >> 31);
    }

    size_t operator()(uint64_t x) const {
        static const uint64_t seed =
            chrono::steady_clock::now().time_since_epoch().count();
        return splitmix64(x + seed);
    }
};

struct State {
    int width;
    long long height;
    i128 cost;
};

static void printInt128(i128 x) {
    if (x == 0) {
        cout << "0\n";
        return;
    }

    if (x < 0) {
        cout << '-';
        x = -x;
    }

    string s;
    while (x > 0) {
        s.push_back(char('0' + x % 10));
        x /= 10;
    }
    reverse(s.begin(), s.end());
    cout << s << '\n';
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    long long K;
    cin >> N >> K;

    vector<long long> H(N), C(N);
    i128 sumOriginalHeights = 0;

    for (int i = 0; i < N; ++i) {
        cin >> H[i] >> C[i];
        sumOriginalHeights += H[i];
    }

    if (N == 1) {
        cout << "0\n";
        return 0;
    }

    vector<int> order(N);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        if (H[a] != H[b]) return H[a] < H[b];
        return C[a] < C[b];
    });

    vector<long long> sortedH(N);
    vector<long long> prefixMinC(N);

    for (int i = 0; i < N; ++i) {
        sortedH[i] = H[order[i]];
        prefixMinC[i] = C[order[i]];
        if (i > 0) {
            prefixMinC[i] = min(prefixMinC[i], prefixMinC[i - 1]);
        }
    }

    auto available = [&](long long height) -> int {
        return int(upper_bound(sortedH.begin(), sortedH.end(), height)
                   - sortedH.begin());
    };

    auto cheapestAvailable = [&](long long height) -> long long {
        int count = available(height);
        return prefixMinC[count - 1];
    };

    map<long long, long long> bestRootCost;
    for (int i = 0; i < N; ++i) {
        auto it = bestRootCost.find(H[i]);
        if (it == bestRootCost.end()) {
            bestRootCost[H[i]] = C[i];
        } else {
            it->second = min(it->second, C[i]);
        }
    }

    vector<unordered_map<u64, i128, CustomHash>> dp(N + 1);

    auto relax = [&](int scheduled, long long height, int width, i128 cost) {
        u64 key = (u64(height) << SHIFT) | u64(width);
        auto it = dp[scheduled].find(key);
        if (it == dp[scheduled].end() || cost < it->second) {
            dp[scheduled][key] = cost;
        }
    };

    i128 answer = INF;

    /*
      The hotel is the only point on the first altitude level.

      For every distinct possible hotel altitude, using the point with minimum
      extension cost is optimal. The next occupied level is either one metre
      higher or the altitude at which another point becomes available.
    */
    for (const auto &entry : bestRootCost) {
        long long rootHeight = entry.first;
        long long rootCost = entry.second;

        long long nextHeight = max(rootHeight + 1, sortedH[1]);
        int backlog = available(nextHeight) - 1;

        for (int groupSize = 1; groupSize <= backlog; ++groupSize) {
            int scheduled = 1 + groupSize;
            int width = groupSize;

            i128 cost = i128(K) * rootHeight;
            cost += i128(K) * nextHeight * groupSize;
            cost += i128(groupSize - 1) * rootCost;

            if (scheduled == N) {
                answer = min(answer, cost);
            } else {
                relax(scheduled, nextHeight, width, cost);
            }
        }
    }

    for (int scheduled = 2; scheduled < N; ++scheduled) {
        if (dp[scheduled].empty()) continue;

        vector<State> states;
        states.reserve(dp[scheduled].size());

        for (const auto &entry : dp[scheduled]) {
            u64 key = entry.first;
            states.push_back({
                int(key & MASK),
                long long(key >> SHIFT),
                entry.second
            });
        }

        dp[scheduled].clear();
        dp[scheduled].rehash(0);

        /*
          For equal scheduled-count and width, an earlier state with no larger
          cost dominates a later state.
        */
        sort(states.begin(), states.end(), [](const State &a, const State &b) {
            if (a.width != b.width) return a.width < b.width;
            if (a.height != b.height) return a.height < b.height;
            return a.cost < b.cost;
        });

        vector<State> useful;
        useful.reserve(states.size());

        int currentWidth = -1;
        i128 bestCost = INF;

        for (const State &state : states) {
            if (state.width != currentWidth) {
                currentWidth = state.width;
                bestCost = INF;
            }

            if (state.cost < bestCost) {
                useful.push_back(state);
                bestCost = state.cost;
            }
        }

        for (const State &state : useful) {
            long long nextHeight =
                max(state.height + 1, sortedH[scheduled]);

            int backlog = available(nextHeight) - scheduled;
            long long extensionPrice = cheapestAvailable(state.height);

            if (backlog <= state.width) {
                int groupSize = backlog;
                int nextScheduled = scheduled + groupSize;

                i128 nextCost =
                    state.cost + i128(K) * nextHeight * groupSize;

                if (nextScheduled == N) {
                    answer = min(answer, nextCost);
                } else {
                    relax(nextScheduled, nextHeight,
                          state.width, nextCost);
                }
            } else {
                for (int groupSize = state.width;
                     groupSize <= backlog;
                     ++groupSize) {
                    int nextScheduled = scheduled + groupSize;

                    i128 nextCost = state.cost;
                    nextCost += i128(K) * nextHeight * groupSize;
                    nextCost +=
                        i128(groupSize - state.width) * extensionPrice;

                    if (nextScheduled == N) {
                        answer = min(answer, nextCost);
                    } else {
                        relax(nextScheduled, nextHeight,
                              groupSize, nextCost);
                    }
                }
            }
        }
    }

    answer -= i128(K) * sumOriginalHeights;
    printInt128(answer);
    return 0;
}
