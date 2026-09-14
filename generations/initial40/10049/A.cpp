#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    cin >> N;

    vector<long long> A(N), B(N);
    for (auto &x : A) cin >> x;
    for (auto &x : B) cin >> x;

    map<long long, long long> total;
    total[A[0]]++;

    // Counts on the already-painted boundary cells.
    for (int i = 1; i < N; ++i) {
        total[A[i]]++;
        total[B[i]]++;
    }

    // Prefix maxima used by rows/columns 2..N in the interior.
    map<long long, long long> freqX, freqY;
    long long mx = A[0], my = B[0];

    for (int i = 1; i < N; ++i) {
        mx = max(mx, A[i]);
        my = max(my, B[i]);
        freqX[mx]++;
        freqY[my]++;
    }

    // For each value v:
    // count pairs where max(X, Y) = v.
    set<long long> values;
    for (const auto &p : freqX) values.insert(p.first);
    for (const auto &p : freqY) values.insert(p.first);

    long long xLess = 0, yLess = 0;

    for (long long v : values) {
        long long fx = freqX[v];
        long long fy = freqY[v];

        // X=v, Y<=v  plus  Y=v, X<v
        total[v] += fx * (yLess + fy) + fy * xLess;

        xLess += fx;
        yLess += fy;
    }

    long long bestColor = -1, bestCount = -1;
    for (const auto &p : total) {
        if (p.second > bestCount ||
            (p.second == bestCount && p.first > bestColor)) {
            bestColor = p.first;
            bestCount = p.second;
        }
    }

    cout << bestColor << ' ' << bestCount << '\n';
    return 0;
}
