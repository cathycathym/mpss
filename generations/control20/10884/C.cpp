#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <vector>
using namespace std;

using int64 = long long;
static const int64 INF = numeric_limits<int64>::max() / 4;

static int64 addCost(int64 x, int64 c) {
    if (x >= INF - c) return INF;
    return x + c;
}

/*
Exact DP when every permitted height is present in vals.
*/
static int64 exactDP(const vector<int64>& a, vector<int64> vals) {
    sort(vals.begin(), vals.end());
    vals.erase(unique(vals.begin(), vals.end()), vals.end());

    const int m = (int)vals.size();
    vector<int64> up(m), down(m), nextUp(m), nextDown(m);
    vector<int64> pref(m), suff(m);

    for (int j = 0; j < m; ++j) {
        up[j] = down[j] = llabs(a[0] - vals[j]);
    }

    for (int i = 1; i < (int)a.size(); ++i) {
        pref[0] = down[0];
        for (int j = 1; j < m; ++j)
            pref[j] = min(pref[j - 1], down[j]);

        suff[m - 1] = up[m - 1];
        for (int j = m - 2; j >= 0; --j)
            suff[j] = min(suff[j + 1], up[j]);

        for (int j = 0; j < m; ++j) {
            int64 bestUp = up[j];       // equality: direction is unchanged
            int64 bestDown = down[j];

            if (j > 0)
                bestUp = min(bestUp, pref[j - 1]);   // x < vals[j]
            if (j + 1 < m)
                bestDown = min(bestDown, suff[j + 1]); // x > vals[j]

            int64 c = llabs(a[i] - vals[j]);
            nextUp[j] = addCost(bestUp, c);
            nextDown[j] = addCost(bestDown, c);
        }

        up.swap(nextUp);
        down.swap(nextDown);
    }

    return min(*min_element(up.begin(), up.end()),
               *min_element(down.begin(), down.end()));
}

/*
Fallback DP over a small locally generated state set.
This branch is not guaranteed exact for unrestricted large instances.
*/
static int64 sparseFallback(const vector<int64>& a) {
    const int n = (int)a.size();

    auto candidates = [&](int i) {
        vector<int64> v;
        int left = max(0, i - 3);
        int right = min(n - 1, i + 3);

        for (int j = left; j <= right; ++j) {
            v.push_back(a[j] - 1);
            v.push_back(a[j]);
            v.push_back(a[j] + 1);
        }

        v.push_back(a.front() - 1);
        v.push_back(a.front());
        v.push_back(a.front() + 1);
        v.push_back(a.back() - 1);
        v.push_back(a.back());
        v.push_back(a.back() + 1);

        sort(v.begin(), v.end());
        v.erase(unique(v.begin(), v.end()), v.end());
        return v;
    };

    vector<int64> oldVals = candidates(0);
    vector<int64> up(oldVals.size()), down(oldVals.size());

    for (int j = 0; j < (int)oldVals.size(); ++j)
        up[j] = down[j] = llabs(a[0] - oldVals[j]);

    for (int i = 1; i < n; ++i) {
        vector<int64> newVals = candidates(i);
        vector<int64> nextUp(newVals.size(), INF);
        vector<int64> nextDown(newVals.size(), INF);

        for (int yid = 0; yid < (int)newVals.size(); ++yid) {
            int64 y = newVals[yid];
            int64 bestUp = INF;
            int64 bestDown = INF;

            for (int xid = 0; xid < (int)oldVals.size(); ++xid) {
                int64 x = oldVals[xid];

                if (x == y) {
                    bestUp = min(bestUp, up[xid]);
                    bestDown = min(bestDown, down[xid]);
                } else if (x < y) {
                    bestUp = min(bestUp, down[xid]);
                } else {
                    bestDown = min(bestDown, up[xid]);
                }
            }

            int64 c = llabs(a[i] - y);
            nextUp[yid] = addCost(bestUp, c);
            nextDown[yid] = addCost(bestDown, c);
        }

        oldVals.swap(newVals);
        up.swap(nextUp);
        down.swap(nextDown);
    }

    return min(*min_element(up.begin(), up.end()),
               *min_element(down.begin(), down.end()));
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

    int64 mn = *min_element(a.begin(), a.end());
    int64 mx = *max_element(a.begin(), a.end());

    vector<int64> vals;

    // Exact bounded-elevation branch.
    if (mx - mn <= 2500) {
        for (int64 x = mn - 1; x <= mx + 1; ++x)
            vals.push_back(x);

        cout << exactDP(a, vals) << '\n';
        return 0;
    }

    // Exact moderate-n branch using the a_j-1, a_j, a_j+1 lemma.
    const int64 operationBudget = 60000000LL;
    int64 candidateBound = 3LL * n;

    if (candidateBound * n <= operationBudget) {
        vals.reserve(3 * n);
        for (int64 x : a) {
            vals.push_back(x - 1);
            vals.push_back(x);
            vals.push_back(x + 1);
        }

        cout << exactDP(a, vals) << '\n';
        return 0;
    }

    cout << sparseFallback(a) << '\n';
    return 0;
}
