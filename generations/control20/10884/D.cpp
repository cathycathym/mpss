#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
const int64 INF = (1LL << 62);

static inline int64 addCost(int64 x, int64 c) {
    return (x >= INF - c) ? INF : x + c;
}

static int64 exactDP(const vector<int64>& a, vector<int64> vals) {
    sort(vals.begin(), vals.end());
    vals.erase(unique(vals.begin(), vals.end()), vals.end());

    const int m = (int)vals.size();
    vector<int64> up(m), down(m), nextUp(m), nextDown(m);
    vector<int64> pref(m), suff(m);

    for (int j = 0; j < m; ++j)
        up[j] = down[j] = llabs(a[0] - vals[j]);

    for (int i = 1; i < (int)a.size(); ++i) {
        pref[0] = down[0];
        for (int j = 1; j < m; ++j)
            pref[j] = min(pref[j - 1], down[j]);

        suff[m - 1] = up[m - 1];
        for (int j = m - 2; j >= 0; --j)
            suff[j] = min(suff[j + 1], up[j]);

        for (int j = 0; j < m; ++j) {
            int64 bestUp = up[j];
            int64 bestDown = down[j];

            if (j > 0)
                bestUp = min(bestUp, pref[j - 1]);
            if (j + 1 < m)
                bestDown = min(bestDown, suff[j + 1]);

            int64 cost = llabs(a[i] - vals[j]);
            nextUp[j] = addCost(bestUp, cost);
            nextDown[j] = addCost(bestDown, cost);
        }

        up.swap(nextUp);
        down.swap(nextDown);
    }

    return min(*min_element(up.begin(), up.end()),
               *min_element(down.begin(), down.end()));
}

static int64 sparseDP(const vector<int64>& a) {
    const int n = (int)a.size();

    vector<int64> sorted = a;
    sort(sorted.begin(), sorted.end());

    vector<int64> anchors;
    const int QUANTILES = 48;
    for (int q = 0; q <= QUANTILES; ++q) {
        int pos = (int)((int64)q * (n - 1) / QUANTILES);
        for (int d = -1; d <= 1; ++d)
            anchors.push_back(sorted[pos] + d);
    }
    sort(anchors.begin(), anchors.end());
    anchors.erase(unique(anchors.begin(), anchors.end()), anchors.end());

    auto candidates = [&](int i) {
        vector<int64> v = anchors;

        auto addAround = [&](int p) {
            if (p < 0 || p >= n) return;
            v.push_back(a[p] - 1);
            v.push_back(a[p]);
            v.push_back(a[p] + 1);
        };

        for (int d = -4; d <= 4; ++d)
            addAround(i + d);

        for (int step = 8; step < n; step <<= 1) {
            addAround(i - step);
            addAround(i + step);
            if (step > n / 2) break;
        }

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
        const int m = (int)oldVals.size();

        vector<int64> pref(m), suff(m);
        pref[0] = down[0];
        for (int j = 1; j < m; ++j)
            pref[j] = min(pref[j - 1], down[j]);

        suff[m - 1] = up[m - 1];
        for (int j = m - 2; j >= 0; --j)
            suff[j] = min(suff[j + 1], up[j]);

        vector<int64> nextUp(newVals.size(), INF);
        vector<int64> nextDown(newVals.size(), INF);

        for (int j = 0; j < (int)newVals.size(); ++j) {
            int64 y = newVals[j];
            int p = lower_bound(oldVals.begin(), oldVals.end(), y) -
                    oldVals.begin();

            int64 bestUp = (p > 0 ? pref[p - 1] : INF);
            int64 bestDown = (p < m ? suff[p] : INF);

            if (p < m && oldVals[p] == y) {
                bestUp = min(bestUp, up[p]);
                bestDown = min(down[p],
                               p + 1 < m ? suff[p + 1] : INF);
            }

            int64 cost = llabs(a[i] - y);
            nextUp[j] = addCost(bestUp, cost);
            nextDown[j] = addCost(bestDown, cost);
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
    for (int64& x : a)
        cin >> x;

    if (n <= 2) {
        cout << 0 << '\n';
        return 0;
    }

    int64 mn = *min_element(a.begin(), a.end());
    int64 mx = *max_element(a.begin(), a.end());

    if (mx - mn <= 1200) {
        vector<int64> vals;
        vals.reserve((size_t)(mx - mn + 3));
        for (int64 x = mn - 1; x <= mx + 1; ++x)
            vals.push_back(x);

        cout << exactDP(a, move(vals)) << '\n';
        return 0;
    }

    vector<int64> vals;
    vals.reserve(3 * n);
    for (int64 x : a) {
        vals.push_back(x - 1);
        vals.push_back(x);
        vals.push_back(x + 1);
    }

    sort(vals.begin(), vals.end());
    vals.erase(unique(vals.begin(), vals.end()), vals.end());

    const int64 WORK_LIMIT = 80000000LL;
    if ((int64)n * (int64)vals.size() <= WORK_LIMIT)
        cout << exactDP(a, move(vals)) << '\n';
    else
        cout << sparseDP(a) << '\n';

    return 0;
}
