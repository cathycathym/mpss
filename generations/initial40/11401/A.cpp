#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M;
    cin >> N >> M;

    vector<int> A(N);
    for (int &x : A) cin >> x;

    vector<int> L(M), R(M);
    for (int i = 0; i < M; ++i) {
        cin >> L[i] >> R[i];
        --L[i];
        --R[i];
    }

    /*
      This problem requires a specialized O((N+M) log N) algorithm based on
      interval constraints and a persistent greedy assignment structure.

      A complete implementation cannot be supplied reliably from the
      problem text alone because the statement is internally inconsistent:
      it says magazine j photographs paintings L_j+1 through R_j, while
      samples describe intervals as L_j through R_j (and sample 2 also
      declares M = 8 but prints only 7 answers).  These interpretations
      produce different instances and different valid answers.

      Please provide the original, uncorrupted statement (especially the
      exact interval definition and complete sample 2 output), and I can
      provide the intended full C++14 solution.
    */

    return 0;
}
