#include <bits/stdc++.h>
using namespace std;

const int INF = 1e9;

struct Data {
    vector<pair<int,int>> byL, byR;
    int base = 1;
    vector<int> mx, mn;
};

int N, M;
vector<int> L, R, answ;
vector<char> activeQ;
vector<Data> segQ;
vector<vector<int>> segPos;

void addQueryData(int node, int nl, int nr, int idx) {
    segQ[node].byL.push_back({L[idx], idx});
    segQ[node].byR.push_back({R[idx], idx});
    if (nl == nr) return;
    int mid = (nl + nr) >> 1;
    if (idx <= mid) addQueryData(node << 1, nl, mid, idx);
    else addQueryData(node << 1 | 1, mid + 1, nr, idx);
}

int rangeMax(const Data &d, int l, int r) {
    int res = -INF;
    for (l += d.base, r += d.base; l < r; l >>= 1, r >>= 1) {
        if (l & 1) res = max(res, d.mx[l++]);
        if (r & 1) res = max(res, d.mx[--r]);
    }
    return res;
}

int rangeMin(const Data &d, int l, int r) {
    int res = INF;
    for (l += d.base, r += d.base; l < r; l >>= 1, r >>= 1) {
        if (l & 1) res = min(res, d.mn[l++]);
        if (r & 1) res = min(res, d.mn[--r]);
    }
    return res;
}

void buildQueryTree(int node) {
    Data &d = segQ[node];
    sort(d.byL.begin(), d.byL.end());
    sort(d.byR.begin(), d.byR.end());

    int sz = (int)d.byL.size();
    while (d.base < sz) d.base <<= 1;
    d.mx.assign(d.base << 1, -INF);
    d.mn.assign(d.base << 1, INF);

    for (int i = 0; i < sz; ++i) {
        d.mx[d.base + i] = R[d.byL[i].second];
        d.mn[d.base + i] = L[d.byR[i].second];
    }
    for (int i = d.base - 1; i >= 1; --i) {
        d.mx[i] = max(d.mx[i << 1], d.mx[i << 1 | 1]);
        d.mn[i] = min(d.mn[i << 1], d.mn[i << 1 | 1]);
    }

    if (node << 1 < (int)segQ.size()) {
        if (!segQ[node << 1].byL.empty()) buildQueryTree(node << 1);
        if (!segQ[node << 1 | 1].byL.empty()) buildQueryTree(node << 1 | 1);
    }
}

void eraseFromData(Data &d, int idx) {
    int p = lower_bound(d.byL.begin(), d.byL.end(), make_pair(L[idx], idx)) - d.byL.begin();
    p += d.base;
    d.mx[p] = -INF;
    for (p >>= 1; p; p >>= 1)
        d.mx[p] = max(d.mx[p << 1], d.mx[p << 1 | 1]);

    p = lower_bound(d.byR.begin(), d.byR.end(), make_pair(R[idx], idx)) - d.byR.begin();
    p += d.base;
    d.mn[p] = INF;
    for (p >>= 1; p; p >>= 1)
        d.mn[p] = min(d.mn[p << 1], d.mn[p << 1 | 1]);
}

void eraseQuery(int node, int nl, int nr, int idx) {
    eraseFromData(segQ[node], idx);
    if (nl == nr) return;
    int mid = (nl + nr) >> 1;
    if (idx <= mid) eraseQuery(node << 1, nl, mid, idx);
    else eraseQuery(node << 1 | 1, mid + 1, nr, idx);
}

bool hasPartial(int node, int x, int y) {
    const Data &d = segQ[node];

    // L in [x+1, y], R >= x
    int a = lower_bound(d.byL.begin(), d.byL.end(), make_pair(x + 1, -INF)) - d.byL.begin();
    int b = upper_bound(d.byL.begin(), d.byL.end(), make_pair(y, INF)) - d.byL.begin();
    if (a < b && rangeMax(d, a, b) >= x) return true;

    // R in [x, y-1], L <= y
    a = lower_bound(d.byR.begin(), d.byR.end(), make_pair(x, -INF)) - d.byR.begin();
    b = upper_bound(d.byR.begin(), d.byR.end(), make_pair(y - 1, INF)) - d.byR.begin();
    if (a < b && rangeMin(d, a, b) <= y) return true;

    return false;
}

int firstPartial(int node, int nl, int nr, int x, int y) {
    if (!hasPartial(node, x, y)) return -1;
    if (nl == nr) return nl;
    int mid = (nl + nr) >> 1;
    int left = firstPartial(node << 1, nl, mid, x, y);
    if (left != -1) return left;
    return firstPartial(node << 1 | 1, mid + 1, nr, x, y);
}

void addPositionInterval(int node, int nl, int nr, int ql, int qr, int idx) {
    if (ql <= nl && nr <= qr) {
        segPos[node].push_back(idx);
        return;
    }
    int mid = (nl + nr) >> 1;
    if (ql <= mid) addPositionInterval(node << 1, nl, mid, ql, qr, idx);
    if (qr > mid) addPositionInterval(node << 1 | 1, mid + 1, nr, ql, qr, idx);
}

void coverAtPosition(int node, int nl, int nr, int pos, int value) {
    vector<int> here;
    here.swap(segPos[node]);

    for (int idx : here) {
        if (!activeQ[idx]) continue;
        activeQ[idx] = 0;
        answ[idx] = value;
        eraseQuery(1, 1, M, idx);
    }

    if (nl == nr) return;
    int mid = (nl + nr) >> 1;
    if (pos <= mid) coverAtPosition(node << 1, nl, mid, pos, value);
    else coverAtPosition(node << 1 | 1, mid + 1, nr, pos, value);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> N >> M;
    vector<int> A(N);
    for (int &x : A) cin >> x;
    sort(A.rbegin(), A.rend());

    L.assign(M + 1, 0);
    R.assign(M + 1, 0);
    answ.assign(M + 1, 0);
    activeQ.assign(M + 1, 1);

    for (int i = 1; i <= M; ++i) cin >> L[i] >> R[i];

    segQ.resize(4 * M + 5);
    for (int i = 1; i <= M; ++i) addQueryData(1, 1, M, i);
    buildQueryTree(1);

    segPos.resize(4 * N + 5);
    for (int i = 1; i <= M; ++i)
        addPositionInterval(1, 1, N, L[i], R[i], i);

    set<int> freePos;
    for (int i = 1; i <= N; ++i) freePos.insert(i);

    int used = 0;

    for (int i = 1; i <= M; ++i) {
        if (!activeQ[i]) continue;

        int lo = L[i], hi = R[i];

        while (true) {
            auto it1 = freePos.lower_bound(lo);
            auto it2 = freePos.upper_bound(hi);
            int x = *it1;
            int y = *prev(it2);

            int q = firstPartial(1, 1, M, x, y);
            if (q == -1) break;

            lo = max(lo, L[q]);
            hi = min(hi, R[q]);
        }

        int pos = *freePos.lower_bound(lo);
        int value = A[used++];
        freePos.erase(pos);
        coverAtPosition(1, 1, N, pos, value);
    }

    for (int i = 1; i <= M; ++i)
        cout << answ[i] << '\n';
    return 0;
}
