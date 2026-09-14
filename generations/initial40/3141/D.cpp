#include <iostream>
#include <vector>
using namespace std;

using int64 = long long;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, Q;
    int64 S, T;
    cin >> N >> Q >> S >> T;

    vector<int64> diff(N + 1);
    int64 previous, current;
    cin >> previous;  // A_0

    for (int i = 1; i <= N; ++i) {
        cin >> current;
        diff[i] = current - previous;
        previous = current;
    }

    auto contribution = [&](int64 heightChange) -> int64 {
        if (heightChange > 0) {
            return -S * heightChange;
        }
        return -T * heightChange;
    };

    int64 temperature = 0;
    for (int i = 1; i <= N; ++i) {
        temperature += contribution(diff[i]);
    }

    while (Q--) {
        int L, R;
        int64 X;
        cin >> L >> R >> X;

        temperature -= contribution(diff[L]);
        diff[L] += X;
        temperature += contribution(diff[L]);

        if (R < N) {
            temperature -= contribution(diff[R + 1]);
            diff[R + 1] -= X;
            temperature += contribution(diff[R + 1]);
        }

        cout << temperature << '\n';
    }

    return 0;
}