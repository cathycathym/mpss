#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>
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
    for (long long& x : B) cin >> x;

    sort(A.begin(), A.end());
    sort(B.begin(), B.end());

    vector<long long> prefix(N + 1, 0);
    for (int i = 0; i < N; ++i) {
        prefix[i + 1] =
            max(prefix[i], max(A[i].first - B[i], 0LL));
    }

    vector<long long> suffix(N + 1, 0);
    for (int i = N - 1; i >= 0; --i) {
        suffix[i] =
            max(suffix[i + 1], max(A[i + 1].first - B[i], 0LL));
    }

    vector<long long> answer(N + 1);
    for (int removed = 0; removed <= N; ++removed) {
        answer[A[removed].second] =
            max(prefix[removed], suffix[removed]);
    }

    for (int i = 0; i <= N; ++i) {
        if (i) cout << ' ';
        cout << answer[i];
    }
    cout << '\n';

    return 0;
}