#include <algorithm>
#include <cstdint>
#include <iostream>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace std;

using ull = uint64_t;
using ll = long long;

static ull encodePair(uint32_t s, uint32_t v) {
    return (static_cast<ull>(s) << 32) | static_cast<ull>(v);
}

static uint32_t getS(ull x) {
    return static_cast<uint32_t>(x >> 32);
}

static uint32_t getV(ull x) {
    return static_cast<uint32_t>(x & 0xffffffffULL);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M;
    cin >> N >> M;

    vector<pair<uint32_t, uint32_t>> cards(N);
    for (auto &[s, v] : cards) cin >> s >> v;

    vector<pair<uint32_t, uint32_t>> queries(M);
    for (auto &[t, w] : queries) cin >> t >> w;

    unordered_set<ull> attainable;

    if (N <= 20) {
        vector<vector<unordered_set<ull>>> dp(
            N, vector<unordered_set<ull>>(N)
        );

        for (int i = 0; i < N; ++i) {
            dp[i][i].insert(encodePair(cards[i].first, cards[i].second));
        }

        for (int len = 2; len <= N; ++len) {
            for (int l = 0; l + len <= N; ++l) {
                int r = l + len - 1;
                auto &cur = dp[l][r];
                cur.reserve(static_cast<size_t>(len) * len * 2);

                for (int k = l; k < r; ++k) {
                    const auto &left = dp[l][k];
                    const auto &right = dp[k + 1][r];

                    for (ull x : left) {
                        uint32_t sx = getS(x);
                        uint32_t vx = getV(x);

                        for (ull y : right) {
                            uint32_t sy = getS(y);
                            uint32_t vy = getV(y);

                            cur.insert(encodePair(
                                max(sx, sy),
                                max(vx, vy)
                            ));
                            cur.insert(encodePair(
                                min(sx, sy),
                                min(vx, vy)
                            ));
                        }
                    }
                }
            }
        }

        attainable = std::move(dp[0][N - 1]);
    } else {
        vector<pair<uint32_t, uint32_t>> sorted = cards;
        sort(sorted.begin(), sorted.end());

        bool isChain = true;
        uint32_t largestVOfEarlierStrength = 0;

        for (int i = 0; i < N; ) {
            int j = i;
            uint32_t groupMaximumV = 0;

            while (j < N && sorted[j].first == sorted[i].first) {
                if (sorted[j].second < largestVOfEarlierStrength) {
                    isChain = false;
                }
                groupMaximumV = max(groupMaximumV, sorted[j].second);
                ++j;
            }

            largestVOfEarlierStrength =
                max(largestVOfEarlierStrength, groupMaximumV);
            i = j;
        }

        if (isChain) {
            attainable.reserve(static_cast<size_t>(N) * 2);
            for (auto [s, v] : cards) {
                attainable.insert(encodePair(s, v));
            }
        } else {
            uint32_t minS = cards[0].first;
            uint32_t minV = cards[0].second;
            uint32_t maxS = cards[0].first;
            uint32_t maxV = cards[0].second;

            for (auto [s, v] : cards) {
                minS = min(minS, s);
                minV = min(minV, v);
                maxS = max(maxS, s);
                maxV = max(maxV, v);
            }

            attainable.insert(encodePair(minS, minV));
            attainable.insert(encodePair(maxS, maxV));
        }
    }

    bool first = true;
    for (int j = 0; j < M; ++j) {
        ull wanted = encodePair(queries[j].first, queries[j].second);
        if (attainable.find(wanted) != attainable.end()) {
            if (!first) cout << ' ';
            cout << (j + 1);
            first = false;
        }
    }
    cout << '\n';

    return 0;
}
