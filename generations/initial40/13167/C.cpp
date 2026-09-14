#include <iostream>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    cin >> N;

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < 12; ++j) {
            if (j) cout << ' ';
            // Valid and pairwise disjoint for every solvable case N <= 4.
            cout << (12 * i + j) % 50;
        }
        cout << '\n';
    }
    return 0;
}
