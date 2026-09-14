#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, K;
    cin >> N >> K;

    vector<long long> T(N);
    for (long long& time : T) {
        cin >> time;
    }

    if (K >= N) {
        cout << N << '\n';
        return 0;
    }

    vector<long long> gaps;
    gaps.reserve(N - 1);

    for (int i = 0; i + 1 < N; ++i) {
        gaps.push_back(T[i + 1] - T[i] - 1);
    }

    sort(gaps.begin(), gaps.end());

    long long answer = N;
    int mergedBoundaries = N - K;

    for (int i = 0; i < mergedBoundaries; ++i) {
        answer += gaps[i];
    }

    cout << answer << '\n';
    return 0;
}