#include <bits/stdc++.h>
using namespace std;

struct Person {
    int total, id;
    vector<int> a;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M, K;
    cin >> N >> M >> K;

    vector<Person> p(N);
    for (int i = 0; i < N; ++i) {
        p[i].id = i;
        p[i].total = 0;
        p[i].a.resize(M);
        for (int j = 0; j < M; ++j) {
            cin >> p[i].a[j];
            p[i].total += p[i].a[j];
        }
    }

    sort(p.begin(), p.end(), [](const Person& x, const Person& y) {
        if (x.total != y.total) return x.total > y.total;
        return x.id < y.id;
    });

    vector<int> cap(N - 1);
    for (int i = 0; i + 1 < N; ++i) {
        cap[i] = p[i].total - p[i + 1].total;
        if (p[i].id > p[i + 1].id) --cap[i];
    }

    // reach[h][s] says: exactly h hidden cells can have score sum s.
    auto reachable = [&](const vector<int>& vals, int limit) {
        vector<vector<unsigned long long>> bits(
            M + 1, vector<unsigned long long>((limit + 64) / 64, 0)
        );
        bits[0][0] = 1ULL;

        auto setbit = [&](vector<unsigned long long>& b, int pos) {
            b[pos >> 6] |= 1ULL << (pos & 63);
        };
        auto getbit = [&](const vector<unsigned long long>& b, int pos) {
            return (b[pos >> 6] >> (pos & 63)) & 1ULL;
        };

        for (int v : vals) {
            for (int h = M - 1; h >= 0; --h) {
                for (int s = limit - v; s >= 0; --s) {
                    if (getbit(bits[h], s)) setbit(bits[h + 1], s + v);
                }
            }
        }
        return bits;
    };

    // First contestant: only its hidden score sum matters.
    int right = cap[0];
    auto firstReach = reachable(p[0].a, right);
    vector<int> dp(right + 1, -1000000000);
    for (int h = 0; h <= M; ++h) {
        for (int x = 0; x <= right; ++x) {
            if ((firstReach[h][x >> 6] >> (x & 63)) & 1ULL) {
                dp[x] = max(dp[x], h);
            }
        }
    }

    // Interior contestants.
    for (int i = 1; i + 1 < N; ++i) {
        int left = cap[i - 1];
        right = cap[i];

        vector<int> pref(dp.size());
        pref[0] = dp[0];
        for (int x = 1; x < (int)dp.size(); ++x)
            pref[x] = max(pref[x - 1], dp[x]);

        auto reach = reachable(p[i].a, right);
        vector<int> ndp(right + 1, -1000000000);

        for (int h = 0; h <= M; ++h) {
            for (int x = 0; x <= right; ++x) {
                if (!((reach[h][x >> 6] >> (x & 63)) & 1ULL)) continue;

                int upperUncertainty = K * h - x;
                if (upperUncertainty > left) continue;

                int bestPrevious = pref[left - upperUncertainty];
                if (bestPrevious > -1000000000)
                    ndp[x] = max(ndp[x], bestPrevious + h);
            }
        }
        dp.swap(ndp);
    }

    // Last contestant: only its upper uncertainty matters.
    int left = cap[N - 2];
    vector<int> pref(dp.size());
    pref[0] = dp[0];
    for (int x = 1; x < (int)dp.size(); ++x)
        pref[x] = max(pref[x - 1], dp[x]);

    vector<int> deficits(M);
    for (int j = 0; j < M; ++j) deficits[j] = K - p[N - 1].a[j];

    auto lastReach = reachable(deficits, left);
    int maxHidden = 0;
    for (int h = 0; h <= M; ++h) {
        for (int y = 0; y <= left; ++y) {
            if (!((lastReach[h][y >> 6] >> (y & 63)) & 1ULL)) continue;
            maxHidden = max(maxHidden, pref[left - y] + h);
        }
    }

    cout << N * M - maxHidden << '\n';
    return 0;
}
