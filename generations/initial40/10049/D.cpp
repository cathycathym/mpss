#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    cin >> N;

    vector<long long> A(N), B(N);
    for (long long &x : A) cin >> x;
    for (long long &x : B) cin >> x;

    map<long long, long long> total;

    // Count the initially painted first row and first column.
    total[A[0]]++;
    for (int i = 1; i < N; ++i) {
        total[A[i]]++;
        total[B[i]]++;
    }

    // For i,j >= 1 (zero-based), the cell's color is
    // max(max(A[1..i]), max(B[1..j])).
    map<long long, long long> freqA, freqB;
    long long prefixA = A[1];
    long long prefixB = B[1];

    for (int i = 1; i < N; ++i) {
        if (i > 1) {
            prefixA = max(prefixA, A[i]);
            prefixB = max(prefixB, B[i]);
        }
        ++freqA[prefixA];
        ++freqB[prefixB];
    }

    set<long long> values;
    for (const auto &entry : freqA) values.insert(entry.first);
    for (const auto &entry : freqB) values.insert(entry.first);

    long long lessA = 0;
    long long lessB = 0;

    for (long long value : values) {
        long long equalA = freqA[value];
        long long equalB = freqB[value];

        // Pairs whose maximum is exactly value:
        // A-prefix = value, B-prefix <= value, or
        // B-prefix = value, A-prefix < value.
        total[value] += equalA * (lessB + equalB)
                      + equalB * lessA;

        lessA += equalA;
        lessB += equalB;
    }

    long long bestColor = -1;
    long long bestCount = -1;

    for (const auto &[color, count] : total) {
        if (count > bestCount ||
            (count == bestCount && color > bestColor)) {
            bestColor = color;
            bestCount = count;
        }
    }

    cout << bestColor << ' ' << bestCount << '\n';
    return 0;
}