#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    cin >> N;

    vector<long long> A(N + 1), prefixGain(N + 1, 0);
    for (int i = 1; i <= N; ++i) {
        cin >> A[i];
    }

    for (int i = 1; i <= N; ++i) {
        long long B;
        cin >> B;
        prefixGain[i] = prefixGain[i - 1] + B;
    }

    vector<long long> adjustedNeed(N + 1);
    for (int i = 1; i <= N; ++i) {
        adjustedNeed[i] = A[i] - prefixGain[i - 1];
    }

    const long long NEG_INF = numeric_limits<long long>::lowest() / 4;
    vector<long long> prefixMax(N + 1, NEG_INF);
    vector<long long> suffixMax(N + 2, NEG_INF);

    for (int i = 1; i <= N; ++i) {
        prefixMax[i] = max(prefixMax[i - 1], adjustedNeed[i]);
    }
    for (int i = N; i >= 1; --i) {
        suffixMax[i] = max(suffixMax[i + 1], adjustedNeed[i]);
    }

    const long long totalGain = prefixGain[N];
    long long answer = numeric_limits<long long>::max();

    for (int start = 1; start <= N; ++start) {
        long long required = 0;

        required = max(
            required,
            prefixGain[start - 1] + suffixMax[start]
        );

        if (start > 1) {
            required = max(
                required,
                prefixGain[start - 1] - totalGain + prefixMax[start - 1]
            );
        }

        answer = min(answer, required);
    }

    cout << answer << '\n';
    return 0;
}