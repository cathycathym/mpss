#include <bits/stdc++.h>
using namespace std;

using ll = long long;

struct Interval {
    ll start, finish;
    bool operator<(const Interval& other) const {
        return start < other.start;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    ll M;
    cin >> N >> M;

    vector<ll> s(N), len(N);
    vector<int> overnight;

    for (int i = 0; i < N; ++i) {
        ll e;
        cin >> s[i] >> e;
        len[i] = (e - s[i] + M) % M;
        if (s[i] > e) overnight.push_back(i);
    }

    // No interval covers minute 0, so a full circular cover is impossible.
    if (overnight.empty()) {
        cout << -1 << '\n';
        return 0;
    }

    // Two consecutive copies suffice for every relevant greedy step.
    vector<Interval> intervals;
    intervals.reserve(2 * N);
    vector<ll> endpoints;
    endpoints.reserve(2 * N);

    for (int i = 0; i < N; ++i) {
        for (int k = 0; k <= 1; ++k) {
            ll a = s[i] + k * M;
            ll b = a + len[i];
            intervals.push_back({a, b});
            endpoints.push_back(b);
        }
    }

    sort(intervals.begin(), intervals.end());
    sort(endpoints.begin(), endpoints.end());
    endpoints.erase(unique(endpoints.begin(), endpoints.end()), endpoints.end());

    int K = (int)endpoints.size();
    vector<int> nextState(K, -1);

    // For each endpoint x, find the furthest endpoint of an interval
    // whose start is at most x.
    int ptr = 0;
    ll bestEnd = -1;

    for (int id = 0; id < K; ++id) {
        ll x = endpoints[id];

        // Targets are always below 2M. States at or beyond 2M need not
        // be expanded further.
        if (x >= 2 * M) continue;

        while (ptr < (int)intervals.size() && intervals[ptr].start <= x) {
            bestEnd = max(bestEnd, intervals[ptr].finish);
            ++ptr;
        }

        if (bestEnd > x) {
            int to = (int)(lower_bound(endpoints.begin(), endpoints.end(), bestEnd)
                           - endpoints.begin());
            nextState[id] = to;
        }
    }

    const int LOG = 20; // 2^19 > 2 * 10^5
    vector<vector<int>> up(LOG, vector<int>(K, -1));
    up[0] = nextState;

    for (int p = 1; p < LOG; ++p) {
        for (int i = 0; i < K; ++i) {
            if (up[p - 1][i] != -1) {
                up[p][i] = up[p - 1][up[p - 1][i]];
            }
        }
    }

    int answer = INT_MAX;

    for (int idx : overnight) {
        // This interval is [s[idx], s[idx] + len[idx]] and crosses M.
        // After it ends, cover until its next-day start.
        ll firstEnd = s[idx] + len[idx];
        ll target = s[idx] + M;

        int state = (int)(lower_bound(endpoints.begin(), endpoints.end(), firstEnd)
                          - endpoints.begin());

        int added = 0;

        // Take the maximum number of greedy transitions that still leaves
        // us strictly before the target.
        for (int p = LOG - 1; p >= 0; --p) {
            int candidate = up[p][state];
            if (candidate != -1 && endpoints[candidate] < target) {
                state = candidate;
                added += (1 << p);
            }
        }

        int finalStep = up[0][state];
        if (finalStep != -1 && endpoints[finalStep] >= target) {
            answer = min(answer, 1 + added + 1);
        }
    }

    cout << (answer == INT_MAX ? -1 : answer) << '\n';
    return 0;
}
