#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    long long T;
    cin >> N >> T;

    vector<long long> residues(N);
    for (long long& r : residues) {
        long long A;
        cin >> A;
        r = A % T;
    }

    sort(residues.begin(), residues.end());

    long long largestGap = residues.front() + T - residues.back();
    for (int i = 1; i < N; ++i) {
        largestGap = max(largestGap, residues[i] - residues[i - 1]);
    }

    long long coveringArcLength = T - largestGap;
    cout << (coveringArcLength + 1) / 2 << '\n';

    return 0;
}