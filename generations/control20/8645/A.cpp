#include <algorithm>
#include <climits>
#include <iostream>
#include <vector>
using namespace std;

struct SegmentTree {
    static const int INF = 1000000000;

    int n, lg;
    vector<int> plus0, minus0, plus1, minus1, lazy;

    SegmentTree(const vector<int>& p0,
                const vector<int>& m0,
                const vector<int>& p1,
                const vector<int>& m1) {
        int length = (int)p0.size();
        n = 1;
        lg = 0;
        while (n < length) {
            n <<= 1;
            ++lg;
        }

        plus0.assign(2 * n, INF);
        minus0.assign(2 * n, INF);
        plus1.assign(2 * n, INF);
        minus1.assign(2 * n, INF);
        lazy.assign(n, 0);

        for (int i = 0; i < length; ++i) {
            plus0[n + i] = p0[i];
            minus0[n + i] = m0[i];
            plus1[n + i] = p1[i];
            minus1[n + i] = m1[i];
        }

        for (int i = n - 1; i >= 1; --i) pull(i);
    }

    void pull(int k) {
        plus0[k] = min(plus0[k << 1], plus0[k << 1 | 1]);
        minus0[k] = min(minus0[k << 1], minus0[k << 1 | 1]);
        plus1[k] = min(plus1[k << 1], plus1[k << 1 | 1]);
        minus1[k] = min(minus1[k << 1], minus1[k << 1 | 1]);
    }

    void applyNode(int k, int delta) {
        plus0[k] += delta;
        plus1[k] += delta;
        minus0[k] -= delta;
        minus1[k] -= delta;
        if (k < n) lazy[k] += delta;
    }

    void push(int k) {
        if (lazy[k] != 0) {
            applyNode(k << 1, lazy[k]);
            applyNode(k << 1 | 1, lazy[k]);
            lazy[k] = 0;
        }
    }

    void rangeAdd(int left, int right, int delta) {
        if (left >= right) return;

        int l = left + n;
        int r = right + n;
        int originalL = l;
        int originalR = r;

        for (int h = lg; h >= 1; --h) {
            if (((originalL >> h) << h) != originalL)
                push(originalL >> h);
            if (((originalR >> h) << h) != originalR)
                push((originalR - 1) >> h);
        }

        while (l < r) {
            if (l & 1) applyNode(l++, delta);
            if (r & 1) applyNode(--r, delta);
            l >>= 1;
            r >>= 1;
        }

        for (int h = 1; h <= lg; ++h) {
            if (((originalL >> h) << h) != originalL)
                pull(originalL >> h);
            if (((originalR >> h) << h) != originalR)
                pull((originalR - 1) >> h);
        }
    }

    bool orientation0Works() const {
        return plus0[1] >= 0 && minus0[1] >= 0;
    }

    bool orientation1Works() const {
        return plus1[1] >= 0 && minus1[1] >= 0;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    cin >> N;

    vector<long long> A(2 * N), B(N), C(N);
    for (long long& x : A) cin >> x;
    for (long long& x : B) cin >> x;
    for (long long& x : C) cin >> x;

    sort(B.begin(), B.end());
    sort(C.begin(), C.end());

    vector<long long> allA = A;
    sort(allA.begin(), allA.end());

    vector<long long> seedValues = allA;
    seedValues.erase(unique(seedValues.begin(), seedValues.end()),
                     seedValues.end());

    vector<int> seedValueId(2 * N);
    for (int i = 0; i < 2 * N; ++i) {
        seedValueId[i] =
            lower_bound(seedValues.begin(), seedValues.end(), A[i])
            - seedValues.begin();
    }

    auto feasible = [&](long long D) -> bool {
        vector<long long> bLowEvent(N), bHighEvent(N);
        vector<long long> cLowEvent(N), cHighEvent(N);

        for (int i = 0; i < N; ++i) {
            // count(B <= x-D) changes at x = B+D.
            bLowEvent[i] = B[i] + D;
            // count(B <= x+D) changes at x = B-D.
            bHighEvent[i] = B[i] - D;

            cLowEvent[i] = C[i] + D;
            cHighEvent[i] = C[i] - D;
        }

        // Merge the five already-sorted event lists.
        vector<long long> coordinates;
        coordinates.reserve(seedValues.size() + 4LL * N);

        size_t pos[5] = {0, 0, 0, 0, 0};
        const vector<long long>* lists[5] = {
            &seedValues, &bLowEvent, &bHighEvent,
            &cLowEvent, &cHighEvent
        };

        while (true) {
            long long nextValue = LLONG_MAX;
            for (int t = 0; t < 5; ++t) {
                if (pos[t] < lists[t]->size()) {
                    nextValue = min(nextValue, (*lists[t])[pos[t]]);
                }
            }
            if (nextValue == LLONG_MAX) break;

            coordinates.push_back(nextValue);
            for (int t = 0; t < 5; ++t) {
                while (pos[t] < lists[t]->size() &&
                       (*lists[t])[pos[t]] == nextValue) {
                    ++pos[t];
                }
            }
        }

        vector<int> coordinateOfSeed(seedValues.size());
        size_t cp = 0;
        for (size_t i = 0; i < seedValues.size(); ++i) {
            while (coordinates[cp] < seedValues[i]) ++cp;
            coordinateOfSeed[i] = (int)cp;
        }

        int M = (int)coordinates.size();
        vector<int> p0(M), m0(M), p1(M), m1(M);

        int countAll = 0;
        int countWindow = 0;
        int countBLow = 0, countBHigh = 0;
        int countCLow = 0, countCHigh = 0;

        // Initially the window is A[0..N-1], which is sorted.
        for (int i = 0; i < M; ++i) {
            long long x = coordinates[i];

            while (countAll < 2 * N && allA[countAll] <= x)
                ++countAll;
            while (countWindow < N && A[countWindow] <= x)
                ++countWindow;
            while (countBLow < N && bLowEvent[countBLow] <= x)
                ++countBLow;
            while (countBHigh < N && bHighEvent[countBHigh] <= x)
                ++countBHigh;
            while (countCLow < N && cLowEvent[countCLow] <= x)
                ++countCLow;
            while (countCHigh < N && cHighEvent[countCHigh] <= x)
                ++countCHigh;

            int window = countWindow;
            int complement = countAll - window;

            // Orientation 0: B uses the window, C the complement.
            p0[i] = min(window - countBLow,
                        countCHigh - complement);
            m0[i] = min(countBHigh - window,
                        complement - countCLow);

            // Orientation 1: C uses the window, B the complement.
            p1[i] = min(window - countCLow,
                        countBHigh - complement);
            m1[i] = min(countCHigh - window,
                        complement - countBLow);
        }

        SegmentTree tree(p0, m0, p1, m1);

        if (tree.orientation0Works() || tree.orientation1Works())
            return true;

        for (int l = 0; l < N; ++l) {
            long long leaving = A[l];
            long long entering = A[l + N];

            if (leaving != entering) {
                int leavingCoordinate =
                    coordinateOfSeed[seedValueId[l]];
                int enteringCoordinate =
                    coordinateOfSeed[seedValueId[l + N]];

                if (leaving < entering) {
                    // The window CDF decreases on [leaving, entering).
                    tree.rangeAdd(leavingCoordinate,
                                  enteringCoordinate, -1);
                } else {
                    // The window CDF increases on [entering, leaving).
                    tree.rangeAdd(enteringCoordinate,
                                  leavingCoordinate, +1);
                }
            }

            if (tree.orientation0Works() || tree.orientation1Works())
                return true;
        }

        return false;
    };

    long long low = -1;
    long long high = 1000000000LL;

    while (high - low > 1) {
        long long middle = (low + high) / 2;
        if (feasible(middle))
            high = middle;
        else
            low = middle;
    }

    cout << high << '\n';
    return 0;
}
