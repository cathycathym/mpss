#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    long long T;
    cin >> N >> T;

    vector<long long> residues(N);
    for (int i = 0; i < N; ++i) {
        long long A;
        cin >> A;
        residues[i] = A % T;
    }

    sort(residues.begin(), residues.end());

    long long largestGap = 0;

    for (int i = 1; i < N; ++i) {
        largestGap = max(largestGap, residues[i] - residues[i - 1]);
    }

    largestGap = max(largestGap,
                     residues[0] + T - residues[N - 1]);

    long long intervalLength = T - largestGap;
    long long answer = (intervalLength + 1) / 2;

    cout << answer << '\n';
    return 0;
}