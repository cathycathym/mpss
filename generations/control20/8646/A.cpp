#include <bits/stdc++.h>
using namespace std;

struct PairHash {
    size_t operator()(const pair<long long, long long>& p) const {
        uint64_t x = static_cast<uint64_t>(p.first);
        uint64_t y = static_cast<uint64_t>(p.second);
        x ^= x >> 23;
        x *= 0x2127599bf4325c37ULL;
        x ^= x >> 47;
        y ^= y >> 23;
        y *= 0x9e3779b97f4a7c15ULL;
        y ^= y >> 47;
        return static_cast<size_t>(x ^ (y + 0x9e3779b97f4a7c15ULL + (x << 6) + (x >> 2)));
    }
};

static uint16_t withSingle[512][9];

int stateOf(long long x, long long y, long long tx, long long ty) {
    int a = (x < tx ? 0 : (x == tx ? 1 : 2));
    int b = (y < ty ? 0 : (y == ty ? 1 : 2));
    return a * 3 + b;
}

int combineState(int a, int b, bool takeMax) {
    int ax = a / 3, ay = a % 3;
    int bx = b / 3, by = b % 3;
    int cx = takeMax ? max(ax, bx) : min(ax, bx);
    int cy = takeMax ? max(ay, by) : min(ay, by);
    return cx * 3 + cy;
}

uint16_t combineMasks(uint16_t a, uint16_t b) {
    uint16_t result = 0;
    for (int x = 0; x < 9; ++x) {
        if (!(a & (1u << x))) continue;
        for (int y = 0; y < 9; ++y) {
            if (!(b & (1u << y))) continue;
            result |= uint16_t(1u << combineState(x, y, false));
            result |= uint16_t(1u << combineState(x, y, true));
        }
    }
    return result;
}

uint16_t balancedValue(const vector<int>& states, int l, int r) {
    if (l + 1 == r) return uint16_t(1u << states[l]);
    int mid = (l + r) / 2;
    return combineMasks(balancedValue(states, l, mid),
                        balancedValue(states, mid, r));
}

bool solveSmallExact(const vector<int>& states) {
    int n = static_cast<int>(states.size());
    vector<vector<uint16_t>> dp(n, vector<uint16_t>(n));
    for (int i = 0; i < n; ++i) dp[i][i] = uint16_t(1u << states[i]);

    for (int len = 2; len <= n; ++len) {
        for (int l = 0; l + len <= n; ++l) {
            int r = l + len - 1;
            uint16_t cur = 0;
            for (int k = l; k < r; ++k) {
                cur |= combineMasks(dp[l][k], dp[k + 1][r]);
            }
            dp[l][r] = cur;
        }
    }
    return (dp[0][n - 1] & (1u << 4)) != 0;
}

bool solveEndpointDP(const vector<int>& states) {
    int n = static_cast<int>(states.size());
    vector<uint16_t> dp(static_cast<size_t>(n) * n);

    auto at = [&](int l, int r) -> uint16_t& {
        return dp[static_cast<size_t>(l) * n + r];
    };

    for (int i = 0; i < n; ++i) at(i, i) = uint16_t(1u << states[i]);

    for (int len = 2; len <= n; ++len) {
        for (int l = 0; l + len <= n; ++l) {
            int r = l + len - 1;
            uint16_t a = withSingle[at(l, r - 1)][states[r]];
            uint16_t b = withSingle[at(l + 1, r)][states[l]];
            at(l, r) = a | b;
        }
    }

    return (at(0, n - 1) & (1u << 4)) != 0;
}

bool solveLargeFewQueries(
    const vector<long long>& s,
    const vector<long long>& v,
    long long t,
    long long w
) {
    int n = static_cast<int>(s.size());
    vector<int> states(n);
    for (int i = 0; i < n; ++i) states[i] = stateOf(s[i], v[i], t, w);

    uint16_t left = uint16_t(1u << states[0]);
    for (int i = 1; i < n; ++i) left = withSingle[left][states[i]];
    if (left & (1u << 4)) return true;

    uint16_t right = uint16_t(1u << states[n - 1]);
    for (int i = n - 2; i >= 0; --i) right = withSingle[right][states[i]];
    if (right & (1u << 4)) return true;

    if (balancedValue(states, 0, n) & (1u << 4)) return true;

    vector<long long> prefMinS(n + 1, LLONG_MAX);
    vector<long long> prefMinV(n + 1, LLONG_MAX);
    vector<long long> prefMaxS(n + 1, LLONG_MIN);
    vector<long long> prefMaxV(n + 1, LLONG_MIN);
    vector<long long> suffMinS(n + 1, LLONG_MAX);
    vector<long long> suffMinV(n + 1, LLONG_MAX);
    vector<long long> suffMaxS(n + 1, LLONG_MIN);
    vector<long long> suffMaxV(n + 1, LLONG_MIN);

    for (int i = 0; i < n; ++i) {
        prefMinS[i + 1] = min(prefMinS[i], s[i]);
        prefMinV[i + 1] = min(prefMinV[i], v[i]);
        prefMaxS[i + 1] = max(prefMaxS[i], s[i]);
        prefMaxV[i + 1] = max(prefMaxV[i], v[i]);
    }
    for (int i = n - 1; i >= 0; --i) {
        suffMinS[i] = min(suffMinS[i + 1], s[i]);
        suffMinV[i] = min(suffMinV[i + 1], v[i]);
        suffMaxS[i] = max(suffMaxS[i + 1], s[i]);
        suffMaxV[i] = max(suffMaxV[i + 1], v[i]);
    }

    auto comparablePrefix = [&](int len) {
        if (len == 0) return true;
        bool canLow = prefMinS[len] <= t && prefMinV[len] <= w;
        bool canHigh = prefMaxS[len] >= t && prefMaxV[len] >= w;
        return canLow || canHigh;
    };

    auto comparableSuffix = [&](int pos) {
        if (pos == n) return true;
        bool canLow = suffMinS[pos] <= t && suffMinV[pos] <= w;
        bool canHigh = suffMaxS[pos] >= t && suffMaxV[pos] >= w;
        return canLow || canHigh;
    };

    const int K = 80;
    vector<vector<uint16_t>> band(K + 1, vector<uint16_t>(n));

    for (int i = 0; i < n; ++i) band[1][i] = uint16_t(1u << states[i]);

    for (int len = 1; len <= K && len <= n; ++len) {
        if (len >= 2) {
            for (int l = 0; l + len <= n; ++l) {
                int r = l + len - 1;
                band[len][l] =
                    withSingle[band[len - 1][l]][states[r]] |
                    withSingle[band[len - 1][l + 1]][states[l]];
            }
        }

        for (int l = 0; l + len <= n; ++l) {
            if (!(band[len][l] & (1u << 4))) continue;
            if (comparablePrefix(l) && comparableSuffix(l + len)) return true;
        }
    }

    return false;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    for (int mask = 0; mask < 512; ++mask) {
        for (int y = 0; y < 9; ++y) {
            uint16_t result = 0;
            for (int x = 0; x < 9; ++x) {
                if (!(mask & (1 << x))) continue;
                result |= uint16_t(1u << combineState(x, y, false));
                result |= uint16_t(1u << combineState(x, y, true));
            }
            withSingle[mask][y] = result;
        }
    }

    int n, m;
    cin >> n >> m;

    vector<long long> s(n), v(n);
    long long globalMinS = LLONG_MAX, globalMinV = LLONG_MAX;
    long long globalMaxS = LLONG_MIN, globalMaxV = LLONG_MIN;

    unordered_set<pair<long long, long long>, PairHash> original;
    original.reserve(static_cast<size_t>(n) * 2 + 10);

    vector<pair<long long, long long>> byStrength(n);

    for (int i = 0; i < n; ++i) {
        cin >> s[i] >> v[i];
        globalMinS = min(globalMinS, s[i]);
        globalMinV = min(globalMinV, v[i]);
        globalMaxS = max(globalMaxS, s[i]);
        globalMaxV = max(globalMaxV, v[i]);
        original.insert({s[i], v[i]});
        byStrength[i] = {s[i], v[i]};
    }

    vector<pair<long long, long long>> queries(m);
    for (auto& q : queries) cin >> q.first >> q.second;

    sort(byStrength.begin(), byStrength.end());

    vector<long long> uniqueS;
    vector<long long> groupMaxV;
    vector<long long> groupMinV;

    for (int i = 0; i < n;) {
        int j = i;
        long long mx = LLONG_MIN, mn = LLONG_MAX;
        while (j < n && byStrength[j].first == byStrength[i].first) {
            mx = max(mx, byStrength[j].second);
            mn = min(mn, byStrength[j].second);
            ++j;
        }
        uniqueS.push_back(byStrength[i].first);
        groupMaxV.push_back(mx);
        groupMinV.push_back(mn);
        i = j;
    }

    int g = static_cast<int>(uniqueS.size());
    vector<long long> prefixMaxV(g + 1, LLONG_MIN);
    vector<long long> suffixMinV(g + 1, LLONG_MAX);

    for (int i = 0; i < g; ++i)
        prefixMaxV[i + 1] = max(prefixMaxV[i], groupMaxV[i]);
    for (int i = g - 1; i >= 0; --i)
        suffixMinV[i] = min(suffixMinV[i + 1], groupMinV[i]);

    vector<int> answer;

    for (int qi = 0; qi < m; ++qi) {
        long long t = queries[qi].first;
        long long w = queries[qi].second;
        bool possible = false;

        if ((t == globalMinS && w == globalMinV) ||
            (t == globalMaxS && w == globalMaxV)) {
            possible = true;
        }

        if (!possible && original.count({t, w})) {
            int leftGroups = int(lower_bound(uniqueS.begin(), uniqueS.end(), t)
                                 - uniqueS.begin());
            int firstGreater = int(upper_bound(uniqueS.begin(), uniqueS.end(), t)
                                   - uniqueS.begin());

            bool noNorthWest =
                prefixMaxV[leftGroups] == LLONG_MIN ||
                prefixMaxV[leftGroups] <= w;
            bool noSouthEast =
                suffixMinV[firstGreater] == LLONG_MAX ||
                suffixMinV[firstGreater] >= w;

            if (noNorthWest && noSouthEast) possible = true;
        }

        if (!possible && m <= 10) {
            vector<int> states(n);
            for (int i = 0; i < n; ++i)
                states[i] = stateOf(s[i], v[i], t, w);

            if (n <= 20)
                possible = solveSmallExact(states);
            else if (n <= 2200)
                possible = solveEndpointDP(states);
            else
                possible = solveLargeFewQueries(s, v, t, w);
        }

        if (possible) answer.push_back(qi + 1);
    }

    for (int i = 0; i < static_cast<int>(answer.size()); ++i) {
        if (i) cout << ' ';
        cout << answer[i];
    }
    cout << '\n';
    return 0;
}
