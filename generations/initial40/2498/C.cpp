#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>

using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    cin >> N;

    vector<pair<long long, int>> a(N + 1);
    for (int i = 0; i <= N; ++i) {
        cin >> a[i].first;
        a[i].second = i;
    }

    vector<long long> b(N);
    for (long long &x : b) {
        cin >> x;
    }

    sort(a.begin(), a.end());
    sort(b.begin(), b.end());

    vector<long long> pref(N + 1, 0);
    for (int i = 0; i < N; ++i) {
        long long strangeness = max(a[i].first - b[i], 0LL);
        pref[i + 1] = max(pref[i], strangeness);
    }

    vector<long long> suff(N + 1, 0);
    for (int i = N - 1; i >= 0; --i) {
        long long strangeness = max(a[i + 1].first - b[i], 0LL);
        suff[i] = max(suff[i + 1], strangeness);
    }

    vector<long long> answer(N + 1);
    for (int k = 0; k <= N; ++k) {
        answer[a[k].second] = max(pref[k], suff[k]);
    }

    for (int i = 0; i <= N; ++i) {
        if (i > 0) cout << ' ';
        cout << answer[i];
    }
    cout << '\n';

    return 0;
}