#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    cin >> N;

    vector<long long> A(N), B(N), P(N), Q(N);
    for (int i = 0; i < N; ++i) cin >> A[i];
    for (int i = 0; i < N; ++i) cin >> B[i];

    P[0] = A[0];
    Q[0] = B[0];
    for (int i = 1; i < N; ++i) {
        P[i] = max(P[i - 1], A[i]);
        Q[i] = max(Q[i - 1], B[i]);
    }

    long long bestColor = -1, bestCount = -1;
    long long lessP = 0, lessQ = 0;
    int i = 0, j = 0;

    while (i < N || j < N) {
        long long x;
        if (j == N || (i < N && P[i] < Q[j])) x = P[i];
        else if (i == N || Q[j] < P[i]) x = Q[j];
        else x = P[i];

        long long cntP = 0, cntQ = 0;
        while (i < N && P[i] == x) ++cntP, ++i;
        while (j < N && Q[j] == x) ++cntQ, ++j;

        long long count = cntP * (lessQ + cntQ) + cntQ * lessP;

        if (count > bestCount || (count == bestCount && x > bestColor)) {
            bestCount = count;
            bestColor = x;
        }

        lessP += cntP;
        lessQ += cntQ;
    }

    cout << bestColor << ' ' << bestCount << '\n';
    return 0;
}
