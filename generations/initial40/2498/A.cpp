#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    cin >> N;

    vector<pair<long long, int>> A(N + 1);
    for (int i = 0; i <= N; ++i) {
        cin >> A[i].first;
        A[i].second = i;
    }

    vector<long long> B(N);
    for (long long &x : B) cin >> x;

    sort(A.begin(), A.end());
    sort(B.begin(), B.end());

    // prefix[i] is the maximum cost among positions 0 through i.
    vector<long long> prefix(N);
    for (int i = 0; i < N; ++i) {
        long long cost = max(A[i].first - B[i], 0LL);
        prefix[i] = cost;
        if (i > 0) prefix[i] = max(prefix[i], prefix[i - 1]);
    }

    // suffix[i] is the maximum cost among positions i through N-1
    // when those employees receive A[j + 1].
    vector<long long> suffix(N);
    for (int i = N - 1; i >= 0; --i) {
        long long cost = max(A[i + 1].first - B[i], 0LL);
        suffix[i] = cost;
        if (i + 1 < N) suffix[i] = max(suffix[i], suffix[i + 1]);
    }

    vector<long long> answer(N + 1);

    for (int removed = 0; removed <= N; ++removed) {
        long long oddity = 0;

        if (removed > 0) {
            oddity = max(oddity, prefix[removed - 1]);
        }
        if (removed < N) {
            oddity = max(oddity, suffix[removed]);
        }

        answer[A[removed].second] = oddity;
    }

    for (int i = 0; i <= N; ++i) {
        if (i > 0) cout << ' ';
        cout << answer[i];
    }
    cout << '\n';

    return 0;
}