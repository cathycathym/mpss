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

    vector<long long> need(N + 1);
    for (int i = 1; i <= N; ++i) {
        // Required initial strength if monster i were reached
        // after defeating monsters 1..i-1 normally.
        need[i] = A[i] - pref[i - 1];
    }

    vector<long long> suffixMax(N + 2, LLONG_MIN);
    for (int i = N; i >= 1; --i) {
        suffixMax[i] = max(suffixMax[i + 1], need[i]);
    }

    long long answer = LLONG_MAX;
    long long prefixMax = LLONG_MIN;
    long long totalGain = pref[N];

    for (int j = 1; j <= N; ++j) {
        // Monsters j..N are defeated before wrapping around.
        long long required = pref[j - 1] + suffixMax[j];

        // Then monsters 1..j-1 are defeated after gaining totalGain - pref[j-1].
        if (j >= 2) {
            required = max(required,
                           pref[j - 1] - totalGain + prefixMax);
        }

        answer = min(answer, required);
        prefixMax = max(prefixMax, need[j]);
    }

    cout << answer << '\n';
    return 0;
}
