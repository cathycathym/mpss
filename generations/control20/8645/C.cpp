#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

struct CutConstraints {
    int n;
    int left_bound;
    int right_bound;
    bool impossible;
    vector<int> difference;

    explicit CutConstraints(int n_)
        : n(n_), left_bound(0), right_bound(n_),
          impossible(false), difference(n_ + 2, 0) {}

    // Intersect the valid cuts with the set where
    // min(x,a) + min(n-x,b) >= k.
    void require_at_least(int a, int b, int k) {
        if (impossible || k <= 0) return;

        int maximum = min(n, a + b);
        if (k > maximum) {
            impossible = true;
            return;
        }

        int lo = max(0, k - b);
        int hi = min(n, n + a - k);

        if (lo > hi) {
            impossible = true;
            return;
        }

        left_bound = max(left_bound, lo);
        right_bound = min(right_bound, hi);
        if (left_bound > right_bound) impossible = true;
    }

    // Forbid all cuts where
    // min(x,a) + min(n-x,b) >= k.
    void forbid_at_least(int a, int b, int k) {
        if (impossible) return;

        if (k <= 0) {
            ++difference[0];
            --difference[n + 1];
            return;
        }

        int maximum = min(n, a + b);
        if (k > maximum) return;

        int lo = max(0, k - b);
        int hi = min(n, n + a - k);

        if (lo <= hi) {
            ++difference[lo];
            --difference[hi + 1];
        }
    }

    vector<char> finish() const {
        vector<char> result(n + 1, false);
        if (impossible) return result;

        int active = 0;
        for (int x = 0; x <= n; ++x) {
            active += difference[x];
            if (left_bound <= x && x <= right_bound && active == 0) {
                result[x] = true;
            }
        }
        return result;
    }
};

vector<char> feasible_cuts(
    const vector<long long>& x_arm,
    const vector<long long>& y_arm,
    const vector<long long>& pots,
    long long distance,
    bool select_prefixes
) {
    int n = static_cast<int>(pots.size());
    CutConstraints constraints(n);

    // Conditions s_i <= p_i + distance:
    // count(selected values <= p_i + distance) >= i.
    int px = 0;
    int py = 0;

    for (int index = 0; index < n; ++index) {
        long long threshold = pots[index] + distance;

        while (px < n && x_arm[px] <= threshold) ++px;
        while (py < n && y_arm[py] <= threshold) ++py;

        int rank = index + 1;

        if (select_prefixes) {
            // Selected count is f(x).
            constraints.require_at_least(px, py, rank);
        } else {
            // Selected suffix count is px + py - f(x).
            // Require px + py - f(x) >= rank,
            // or forbid f(x) >= px + py - rank + 1.
            int k = px + py - rank + 1;
            constraints.forbid_at_least(px, py, k);
        }
    }

    // Conditions s_i >= p_i - distance:
    // count(selected values < p_i - distance) <= i-1.
    px = 0;
    py = 0;

    for (int index = 0; index < n; ++index) {
        long long threshold = pots[index] - distance;

        while (px < n && x_arm[px] < threshold) ++px;
        while (py < n && y_arm[py] < threshold) ++py;

        int rank = index + 1;

        if (select_prefixes) {
            // Require f(x) <= rank-1, so forbid f(x) >= rank.
            constraints.forbid_at_least(px, py, rank);
        } else {
            // Require px + py - f(x) <= rank-1,
            // hence f(x) >= px + py - rank + 1.
            int k = px + py - rank + 1;
            constraints.require_at_least(px, py, k);
        }
    }

    return constraints.finish();
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<long long> a(2 * n);
    vector<long long> red(n);
    vector<long long> blue(n);

    for (long long& value : a) cin >> value;
    for (long long& value : red) cin >> value;
    for (long long& value : blue) cin >> value;

    sort(red.begin(), red.end());
    sort(blue.begin(), blue.end());

    vector<long long> x_arm(n);
    vector<long long> y_arm(n);

    for (int i = 0; i < n; ++i) {
        x_arm[i] = a[i];
        y_arm[i] = a[2 * n - 1 - i];
    }

    auto possible = [&](long long distance) -> bool {
        vector<char> red_prefix =
            feasible_cuts(x_arm, y_arm, red, distance, true);
        vector<char> red_suffix =
            feasible_cuts(x_arm, y_arm, red, distance, false);
        vector<char> blue_prefix =
            feasible_cuts(x_arm, y_arm, blue, distance, true);
        vector<char> blue_suffix =
            feasible_cuts(x_arm, y_arm, blue, distance, false);

        for (int cut = 0; cut <= n; ++cut) {
            if (red_prefix[cut] && blue_suffix[cut]) return true;
            if (blue_prefix[cut] && red_suffix[cut]) return true;
        }
        return false;
    };

    long long low = -1;
    long long high = 1000000000LL;

    while (high - low > 1) {
        long long middle = low + (high - low) / 2;
        if (possible(middle)) {
            high = middle;
        } else {
            low = middle;
        }
    }

    cout << high << '\n';
    return 0;
}
