#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    cin >> N;

    vector<long long> A(N + 1), B(N + 1), pref(N + 1, 0);
    for (int i = 1; i <= N; ++i) cin >> A[i];
    for (int i = 1; i <= N; ++i) {
        cin >> B[i];
        pref[i] = pref[i - 1] + B[i];
    }

    vector<long long> C(N + 1);
    for (int i = 1; i <= N; ++i) {
        C[i] = A[i] - pref[i - 1];
    }

    const long long NEG = -(1LL << 60);
    vector<long long> preMax(N + 1, NEG), sufMax(N + 2, NEG);

    for (int i = 1; i <= N; ++i) {
        preMax[i] = max(preMax[i - 1], C[i]);
    }
    for (int i = N; i >= 1; --i) {
        sufMax[i] = max(sufMax[i + 1], C[i]);
    }

    long long answer = (1LL << 62);

    for (int start = 1; start <= N; ++start) {
        long long need = 0;

        // Monsters start, start+1, ..., N.
        need = max(need, pref[start - 1] + sufMax[start]);

        // Monsters 1, 2, ..., start-1.
        if (start > 1) {
            need = max(need, pref[start - 1] - pref[N] + preMax[start - 1]);
        }

        answer = min(answer, need);
    }

    cout << answer << '\n';
    return 0;
}
