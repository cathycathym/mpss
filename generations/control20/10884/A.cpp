#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
const int64 INF = (1LL << 62);

static int64 constantCostSorted(const vector<int64>& a,
                                const vector<int64>& pref,
                                int l, int r) {
    if (l > r) return 0;
    int mid = (l + r) / 2;
    int64 x = a[mid];

    int64 leftCost = x * (mid - l) - (pref[mid] - pref[l]);
    int64 rightCost = (pref[r + 1] - pref[mid + 1]) - x * (r - mid);
    return leftCost + rightCost;
}

static int64 solveIncreasing(const vector<int64>& a) {
    int n = (int)a.size();
    if (n <= 2) return 0;

    vector<int64> pref(n + 1, 0);
    for (int i = 0; i < n; ++i) pref[i + 1] = pref[i] + a[i];

    int64 answer = constantCostSorted(a, pref, 0, n - 1);

    // A sequence consisting of at most two constant runs never contains a slope.
    for (int split = 1; split < n; ++split) {
        answer = min(answer,
                     constantCostSorted(a, pref, 0, split - 1) +
                     constantCostSorted(a, pref, split, n - 1));
    }

    // Strict alternating sequences. Reversing disjoint increasing pairs is
    // independent, and reversing x < y costs y - x + 1.
    int64 startUp = 0;
    int64 startDown = 0;
    for (int edge = 0; edge + 1 < n; ++edge) {
        int64 reverseCost = a[edge + 1] - a[edge] + 1;
        if (edge % 2 == 1) startUp += reverseCost;
        else startDown += reverseCost;
    }

    answer = min(answer, min(startUp, startDown));
    return answer;
}

static int64 exactDP(const vector<int64>& a, vector<int64> values) {
    sort(values.begin(), values.end());
    values.erase(unique(values.begin(), values.end()), values.end());

    int m = (int)values.size();
    vector<int64> flat(m), up(m, INF), down(m, INF);

    for (int x = 0; x < m; ++x)
        flat[x] = llabs(a[0] - values[x]);

    vector<int64> newFlat(m), newUp(m), newDown(m);
    vector<int64> prefix(m), suffix(m);

    for (int i = 1; i < (int)a.size(); ++i) {
        int64 best = INF;
        for (int x = 0; x < m; ++x) {
            prefix[x] = best;
            best = min(best, min(flat[x], down[x]));
        }

        best = INF;
        for (int x = m - 1; x >= 0; --x) {
            suffix[x] = best;
            best = min(best, min(flat[x], up[x]));
        }

        for (int x = 0; x < m; ++x) {
            int64 add = llabs(a[i] - values[x]);

            newFlat[x] = min(INF, flat[x] + add);
            newUp[x] = min(up[x], prefix[x]);
            newDown[x] = min(down[x], suffix[x]);

            if (newUp[x] < INF - add) newUp[x] += add;
            else newUp[x] = INF;

            if (newDown[x] < INF - add) newDown[x] += add;
            else newDown[x] = INF;
        }

        flat.swap(newFlat);
        up.swap(newUp);
        down.swap(newDown);
    }

    int64 answer = INF;
    for (int x = 0; x < m; ++x)
        answer = min(answer, min(flat[x], min(up[x], down[x])));
    return answer;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<int64> a(n);
    for (int64& x : a) cin >> x;

    if (n <= 2) {
        cout << 0 << '\n';
        return 0;
    }

    bool strictlyIncreasing = true;
    for (int i = 1; i < n; ++i) {
        if (a[i - 1] >= a[i]) {
            strictlyIncreasing = false;
            break;
        }
    }

    vector<int64> values = a;
    sort(values.begin(), values.end());
    values.erase(unique(values.begin(), values.end()), values.end());

    constexpr int64 MAX_DP_WORK = 80000000LL;

    if ((int64)n * (int64)values.size() <= MAX_DP_WORK) {
        cout << exactDP(a, values) << '\n';
    } else if (strictlyIncreasing) {
        cout << solveIncreasing(a) << '\n';
    } else {
        // Always-valid fallback: make every elevation equal to a median.
        vector<int64> copy = a;
        nth_element(copy.begin(), copy.begin() + n / 2, copy.end());
        int64 median = copy[n / 2];
        int64 answer = 0;
        for (int64 x : a) answer += llabs(x - median);
        cout << answer << '\n';
    }

    return 0;
}
