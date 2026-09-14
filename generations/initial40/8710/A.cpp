#include <bits/stdc++.h>
using namespace std;

using ll = long long;

struct Interval {
    ll s, r;
    bool operator<(const Interval& other) const {
        return s < other.s;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    ll M;
    cin >> N >> M;

    vector<Interval> intervals;
    vector<pair<ll, ll>> overnight;
    vector<ll> coords;

    for (int i = 0; i < N; ++i) {
        ll s, e;
        cin >> s >> e;

        ll r = e;
        if (s > e) {
            r += M;
            overnight.push_back({s, e});
        }

        intervals.push_back({s, r});
        coords.push_back(e);
    }

    if (overnight.empty()) {
        cout << -1 << '\n';
        return 0;
    }

    sort(intervals.begin(), intervals.end());
    sort(coords.begin(), coords.end());
    coords.erase(unique(coords.begin(), coords.end()), coords.end());

    int K = (int)coords.size();
    int SENTINEL = K;
    vector<ll> value(K + 1, M);
    for (int i = 0; i < K; ++i) value[i] = coords[i];

    vector<int> nextState(K + 1, -1);
    nextState[SENTINEL] = SENTINEL;

    ll bestEnd = -1;
    int ptr = 0;

    for (int i = 0; i < K; ++i) {
        ll x = coords[i];

        while (ptr < N && intervals[ptr].s <= x) {
            bestEnd = max(bestEnd, intervals[ptr].r);
            ++ptr;
        }

        if (bestEnd <= x) {
            nextState[i] = -1;
        } else if (bestEnd >= M) {
            nextState[i] = SENTINEL;
        } else {
            nextState[i] = lower_bound(coords.begin(), coords.end(), bestEnd) - coords.begin();
        }
    }

    const int LOG = 20;
    vector<vector<int>> up(LOG, vector<int>(K + 1, -1));
    up[0] = nextState;

    for (int j = 1; j < LOG; ++j) {
        for (int i = 0; i <= K; ++i) {
            int mid = up[j - 1][i];
            up[j][i] = (mid == -1 ? -1 : up[j - 1][mid]);
        }
    }

    int answer = INT_MAX;

    for (auto [target, start] : overnight) {
        // This overnight vaidilute already covers [target, M) and [0, start].
        // We only need to greedily cover [start, target].
        int cur = lower_bound(coords.begin(), coords.end(), start) - coords.begin();
        int used = 0;

        for (int j = LOG - 1; j >= 0; --j) {
            int to = up[j][cur];
            if (to != -1 && value[to] < target) {
                cur = to;
                used += (1 << j);
            }
        }

        int to = up[0][cur];
        if (to == -1) continue;

        // One chosen overnight vaidilute plus the greedy linear cover.
        answer = min(answer, 1 + used + 1);
    }

    cout << (answer == INT_MAX ? -1 : answer) << '\n';
    return 0;
}
