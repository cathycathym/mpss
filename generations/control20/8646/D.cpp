#include <bits/stdc++.h>
using namespace std;

using ll = long long;
using Mask = uint16_t;

struct PairHash {
    size_t operator()(const pair<ll, ll>& p) const {
        uint64_t x = static_cast<uint64_t>(p.first);
        uint64_t y = static_cast<uint64_t>(p.second);
        x ^= x >> 23;
        x *= 0x2127599bf4325c37ULL;
        x ^= x >> 47;
        y ^= y >> 23;
        y *= 0x9e3779b97f4a7c15ULL;
        y ^= y >> 47;
        return static_cast<size_t>(
            x ^ (y + 0x9e3779b97f4a7c15ULL + (x << 6) + (x >> 2))
        );
    }
};

static Mask withSingle[512][9];

static int stateOf(ll s, ll v, ll t, ll w) {
    int a = (s < t ? 0 : (s == t ? 1 : 2));
    int b = (v < w ? 0 : (v == w ? 1 : 2));
    return a * 3 + b;
}

static int combineState(int a, int b, bool takeMax) {
    int as = a / 3, av = a % 3;
    int bs = b / 3, bv = b % 3;
    int cs = takeMax ? max(as, bs) : min(as, bs);
    int cv = takeMax ? max(av, bv) : min(av, bv);
    return cs * 3 + cv;
}

static Mask combineMasks(Mask a, Mask b) {
    Mask result = 0;
    for (int x = 0; x < 9; ++x) {
        if (!(a & (Mask(1) << x))) continue;
        for (int y = 0; y < 9; ++y) {
            if (!(b & (Mask(1) << y))) continue;
            result |= Mask(1) << combineState(x, y, false);
            result |= Mask(1) << combineState(x, y, true);
        }
    }
    return result;
}

static bool solveExact(const vector<int>& states) {
    int n = static_cast<int>(states.size());
    vector<vector<Mask> > dp(n, vector<Mask>(n, 0));

    for (int i = 0; i < n; ++i)
        dp[i][i] = Mask(1) << states[i];

    for (int len = 2; len <= n; ++len) {
        for (int l = 0; l + len <= n; ++l) {
            int r = l + len - 1;
            Mask result = 0;
            for (int k = l; k < r; ++k)
                result |= combineMasks(dp[l][k], dp[k + 1][r]);
            dp[l][r] = result;
        }
    }

    return (dp[0][n - 1] & (Mask(1) << 4)) != 0;
}

static bool solveEndpointDP(const vector<int>& states) {
    int n = static_cast<int>(states.size());
    vector<Mask> dp(static_cast<size_t>(n) * n, 0);

    const auto index = [n](int l, int r) -> size_t {
        return static_cast<size_t>(l) * n + r;
    };

    for (int i = 0; i < n; ++i)
        dp[index(i, i)] = Mask(1) << states[i];

    for (int len = 2; len <= n; ++len) {
        for (int l = 0; l + len <= n; ++l) {
            int r = l + len - 1;
            dp[index(l, r)] =
                withSingle[dp[index(l, r - 1)]][states[r]] |
                withSingle[dp[index(l + 1, r)]][states[l]];
        }
    }

    return (dp[index(0, n - 1)] & (Mask(1) << 4)) != 0;
}

static Mask balancedValue(const vector<int>& states, int l, int r) {
    if (r - l == 1)
        return Mask(1) << states[l];

    int mid = (l + r) / 2;
    return combineMasks(
        balancedValue(states, l, mid),
        balancedValue(states, mid, r)
    );
}

static bool additionalSearch(
    const vector<ll>& s,
    const vector<ll>& v,
    const vector<int>& states,
    ll t,
    ll w
) {
    int n = static_cast<int>(s.size());

    Mask current = Mask(1) << states[0];
    for (int i = 1; i < n; ++i)
        current = withSingle[current][states[i]];
    if (current & (Mask(1) << 4)) return true;

    current = Mask(1) << states[n - 1];
    for (int i = n - 2; i >= 0; --i)
        current = withSingle[current][states[i]];
    if (current & (Mask(1) << 4)) return true;

    if (balancedValue(states, 0, n) & (Mask(1) << 4))
        return true;

    vector<ll> prefMinS(n + 1, LLONG_MAX);
    vector<ll> prefMinV(n + 1, LLONG_MAX);
    vector<ll> prefMaxS(n + 1, LLONG_MIN);
    vector<ll> prefMaxV(n + 1, LLONG_MIN);
    vector<ll> suffMinS(n + 1, LLONG_MAX);
    vector<ll> suffMinV(n + 1, LLONG_MAX);
    vector<ll> suffMaxS(n + 1, LLONG_MIN);
    vector<ll> suffMaxV(n + 1, LLONG_MIN);

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

    const auto comparablePrefix = [&](int len) {
        if (len == 0) return true;
        return (prefMinS[len] <= t && prefMinV[len] <= w) ||
               (prefMaxS[len] >= t && prefMaxV[len] >= w);
    };

    const auto comparableSuffix = [&](int pos) {
        if (pos == n) return true;
        return (suffMinS[pos] <= t && suffMinV[pos] <= w) ||
               (suffMaxS[pos] >= t && suffMaxV[pos] >= w);
    };

    const int K = min(n, 64);
    vector<vector<Mask> > band(K + 1);

    band[1].resize(n);
    for (int i = 0; i < n; ++i)
        band[1][i] = Mask(1) << states[i];

    for (int len = 1; len <= K; ++len) {
        if (len >= 2) {
            band[len].resize(n - len + 1);
            for (int l = 0; l + len <= n; ++l) {
                int r = l + len - 1;
                band[len][l] =
                    withSingle[band[len - 1][l]][states[r]] |
                    withSingle[band[len - 1][l + 1]][states[l]];
            }
        }

        for (int l = 0; l + len <= n; ++l) {
            if (!(band[len][l] & (Mask(1) << 4))) continue;
            if (comparablePrefix(l) && comparableSuffix(l + len))
                return true;
        }
    }

    return false;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    for (int mask = 0; mask < 512; ++mask) {
        for (int y = 0; y < 9; ++y) {
            Mask result = 0;
            for (int x = 0; x < 9; ++x) {
                if (!(mask & (1 << x))) continue;
                result |= Mask(1) << combineState(x, y, false);
                result |= Mask(1) << combineState(x, y, true);
            }
            withSingle[mask][y] = result;
        }
    }

    int n, m;
    cin >> n >> m;

    vector<ll> s(n), v(n);
    vector<pair<ll, ll> > sortedCards(n);

    unordered_map<pair<ll, ll>, vector<int>, PairHash> positions;
    positions.reserve(static_cast<size_t>(n) * 2 + 1);

    unordered_set<ll> strengthValues, costValues;
    strengthValues.reserve(static_cast<size_t>(n) * 2 + 1);
    costValues.reserve(static_cast<size_t>(n) * 2 + 1);

    for (int i = 0; i < n; ++i) {
        cin >> s[i] >> v[i];
        sortedCards[i] = make_pair(s[i], v[i]);
        positions[make_pair(s[i], v[i])].push_back(i);
        strengthValues.insert(s[i]);
        costValues.insert(v[i]);
    }

    vector<pair<ll, ll> > queries(m);
    for (int i = 0; i < m; ++i)
        cin >> queries[i].first >> queries[i].second;

    sort(sortedCards.begin(), sortedCards.end());

    vector<ll> distinctS;
    vector<ll> groupMinV;
    vector<ll> groupMaxV;

    for (int i = 0; i < n; ) {
        int j = i;
        ll mn = LLONG_MAX;
        ll mx = LLONG_MIN;

        while (j < n && sortedCards[j].first == sortedCards[i].first) {
            mn = min(mn, sortedCards[j].second);
            mx = max(mx, sortedCards[j].second);
            ++j;
        }

        distinctS.push_back(sortedCards[i].first);
        groupMinV.push_back(mn);
        groupMaxV.push_back(mx);
        i = j;
    }

    int groups = static_cast<int>(distinctS.size());
    vector<ll> prefixMaxV(groups + 1, LLONG_MIN);
    vector<ll> suffixMinV(groups + 1, LLONG_MAX);

    for (int i = 0; i < groups; ++i)
        prefixMaxV[i + 1] = max(prefixMaxV[i], groupMaxV[i]);

    for (int i = groups - 1; i >= 0; --i)
        suffixMinV[i] = min(suffixMinV[i + 1], groupMinV[i]);

    vector<ll> prefMinS(n + 1, LLONG_MAX);
    vector<ll> prefMinV(n + 1, LLONG_MAX);
    vector<ll> prefMaxS(n + 1, LLONG_MIN);
    vector<ll> prefMaxV(n + 1, LLONG_MIN);
    vector<ll> suffMinS(n + 1, LLONG_MAX);
    vector<ll> suffMinV(n + 1, LLONG_MAX);
    vector<ll> suffMaxS(n + 1, LLONG_MIN);
    vector<ll> suffMaxV(n + 1, LLONG_MIN);

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

    unordered_map<pair<ll, ll>, bool, PairHash> originalCache;
    originalCache.reserve(positions.size() * 2 + 1);

    const auto originalCanSurvive = [&](const pair<ll, ll>& q) {
        unordered_map<pair<ll, ll>, bool, PairHash>::const_iterator cached =
            originalCache.find(q);
        if (cached != originalCache.end())
            return cached->second;

        ll t = q.first;
        ll w = q.second;
        bool possible = false;

        const vector<int>& occurrence = positions.find(q)->second;
        for (size_t z = 0; z < occurrence.size() && !possible; ++z) {
            int i = occurrence[z];

            bool leftComparable =
                i == 0 ||
                (prefMinS[i] <= t && prefMinV[i] <= w) ||
                (prefMaxS[i] >= t && prefMaxV[i] >= w);

            bool rightComparable =
                i + 1 == n ||
                (suffMinS[i + 1] <= t && suffMinV[i + 1] <= w) ||
                (suffMaxS[i + 1] >= t && suffMaxV[i + 1] >= w);

            possible = leftComparable && rightComparable;
        }

        originalCache[q] = possible;
        return possible;
    };

    vector<int> answer;

    for (int qi = 0; qi < m; ++qi) {
        ll t = queries[qi].first;
        ll w = queries[qi].second;
        bool possible = false;

        if (strengthValues.find(t) == strengthValues.end() ||
            costValues.find(w) == costValues.end()) {
            continue;
        }

        if (n <= 20) {
            vector<int> states(n);
            for (int i = 0; i < n; ++i)
                states[i] = stateOf(s[i], v[i], t, w);
            possible = solveExact(states);
        } else if (m <= 10 && n <= 2200) {
            vector<int> states(n);
            for (int i = 0; i < n; ++i)
                states[i] = stateOf(s[i], v[i], t, w);
            possible = solveEndpointDP(states);
        } else {
            int firstAtLeast =
                static_cast<int>(lower_bound(
                    distinctS.begin(), distinctS.end(), t
                ) - distinctS.begin());

            int firstGreater =
                static_cast<int>(upper_bound(
                    distinctS.begin(), distinctS.end(), t
                ) - distinctS.begin());

            bool southeast =
                firstAtLeast < groups && suffixMinV[firstAtLeast] <= w;

            bool northwest =
                firstGreater > 0 && prefixMaxV[firstGreater] >= w;

            if (southeast && northwest) {
                pair<ll, ll> q = make_pair(t, w);
                unordered_map<pair<ll, ll>, vector<int>, PairHash>::const_iterator it =
                    positions.find(q);

                if (it == positions.end())
                    possible = true;
                else
                    possible = originalCanSurvive(q);
            }

            if (!possible && m <= 10) {
                vector<int> states(n);
                for (int i = 0; i < n; ++i)
                    states[i] = stateOf(s[i], v[i], t, w);
                possible = additionalSearch(s, v, states, t, w);
            }
        }

        if (possible)
            answer.push_back(qi + 1);
    }

    for (size_t i = 0; i < answer.size(); ++i) {
        if (i) cout << ' ';
        cout << answer[i];
    }
    cout << '\n';

    return 0;
}
