#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, K;
    cin >> N >> K;

    vector<long long> T(N);
    for (long long &x : T) {
        cin >> x;
    }

    vector<long long> gaps;
    for (int i = 0; i + 1 < N; ++i) {
        gaps.push_back(T[i + 1] - T[i] - 1);
    }

    sort(gaps.rbegin(), gaps.rend());

    long long answer = T.back() + 1 - T.front();
    int splits = min(K - 1, N - 1);

    for (int i = 0; i < splits; ++i) {
        answer -= gaps[i];
    }

    cout << answer << '\n';
    return 0;
}