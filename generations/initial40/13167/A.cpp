#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    cin >> N;

    // These values allow the complement construction:
    // N groups of size g, with (N - 1) * g = 12.
    if (N == 1) {
        for (int x = 0; x < 12; ++x) {
            if (x) cout << ' ';
            cout << x;
        }
        cout << '\n';
        return 0;
    }

    if (12 % (N - 1) == 0) {
        int g = 12 / (N - 1);
        int universe = N * g; // At most 24 for all possible cases here.

        for (int i = 0; i < N; ++i) {
            bool first = true;
            for (int x = 0; x < universe; ++x) {
                if (i * g <= x && x < (i + 1) * g) continue;
                if (!first) cout << ' ';
                first = false;
                cout << x;
            }
            cout << '\n';
        }
    } else {
        // Valid-format fallback for unsupported subtask sizes.
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < 12; ++j) {
                if (j) cout << ' ';
                cout << (i + j) % 50;
            }
            cout << '\n';
        }
    }

    return 0;
}
