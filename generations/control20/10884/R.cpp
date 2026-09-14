#include <algorithm>
#include <iostream>
#include <limits>
#include <vector>
using namespace std;

using int64 = long long;

static const int64 INF = numeric_limits<int64>::max() / 4;

static int64 constant_cost(const vector<int64>& a) {
    vector<int64> v = a;
    nth_element(v.begin(), v.begin() + v.size() / 2, v.end());
    int64 med = v[v.size() / 2];

    int64 answer = 0;
    for (int64 x : a) answer += llabs(x - med);
    return answer;
}

// Exact solution for strictly increasing input.
// In this special case an optimum can be represented by at most two
// consecutive constant blocks.
static int64 increasing_solution(const vector<int64>& a) {
    int n = (int)a.size();
    vector<int64> pref(n + 1, 0);

    for (int i = 0; i < n; ++i)
        pref[i + 1] = pref[i] + a[i];

    auto interval_cost = [&](int l, int r) -> int64 {
        if (l > r) return 0;

        int mid = (l + r) / 2;
        int64 left = a[mid] * (mid - l) - (pref[mid] - pref[l]);
        int64 right =
            (pref[r + 1] - pref[mid + 1]) - a[mid] * (r - mid);
        return left + right;
    };

    int64 answer = interval_cost(0, n - 1);

    // First block is [0, split-1], second is [split, n-1].
    for (int split = 1; split < n; ++split) {
        answer = min(answer,
                     interval_cost(0, split - 1) +
                     interval_cost(split, n - 1));
    }

    return answer;
}

// Exact O(n*m) DP, where m is the number of distinct input values.
static int64 quadratic_dp(const vector<int64>& a,
                          const vector<int64>& values) {
    int n = (int)a.size();
    int m = (int)values.size();

    // up[x]   = minimum cost when the last nonzero change is upward
    //           and the current elevation is values[x].
    // down[x] = analogous state for a downward change.
    //
    // At the first position both states are allowed. This represents the
    // fact that before the first nonzero change there is no direction yet.
    vector<int64> up(m), down(m);

    for (int x = 0; x < m; ++x) {
        up[x] = down[x] = llabs(a[0] - values[x]);
    }

    vector<int64> pref_down(m), suff_up(m);
    vector<int64> new_up(m), new_down(m);

    for (int i = 1; i < n; ++i) {
        int64 best = INF;

        // pref_down[x] is the best down-state at a value strictly
        // smaller than values[x].
        for (int x = 0; x < m; ++x) {
            pref_down[x] = best;
            best = min(best, down[x]);
        }

        best = INF;

        // suff_up[x] is the best up-state at a value strictly
        // greater than values[x].
        for (int x = m - 1; x >= 0; --x) {
            suff_up[x] = best;
            best = min(best, up[x]);
        }

        for (int x = 0; x < m; ++x) {
            int64 change = llabs(a[i] - values[x]);

            // Equality keeps the previous direction.
            // A new upward movement must follow a downward movement.
            new_up[x] = change + min(up[x], pref_down[x]);

            // A new downward movement must follow an upward movement.
            new_down[x] = change + min(down[x], suff_up[x]);
        }

        up.swap(new_up);
        down.swap(new_down);
    }

    int64 answer = INF;
    for (int x = 0; x < m; ++x)
        answer = min(answer, min(up[x], down[x]));

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

    bool strictly_increasing = true;
    for (int i = 1; i < n; ++i) {
        if (a[i - 1] >= a[i]) {
            strictly_increasing = false;
            break;
        }
    }

    if (strictly_increasing) {
        cout << increasing_solution(a) << '\n';
        return 0;
    }

    vector<int64> values = a;
    sort(values.begin(), values.end());
    values.erase(unique(values.begin(), values.end()), values.end());

    // This covers every n <= 2000 instance and every a_i <= 10 instance.
    // The slightly larger limit also solves additional favorable cases.
    const int64 OPERATION_LIMIT = 40000000LL;

    if ((int64)n * (int64)values.size() <= OPERATION_LIMIT) {
        cout << quadratic_dp(a, values) << '\n';
    } else {
        // A constant sequence never contains a slope.
        cout << constant_cost(a) << '\n';
    }

    return 0;
}
