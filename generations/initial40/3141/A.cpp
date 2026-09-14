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

    vector<int64> altitude(N + 1);
    for (int i = 0; i <= N; ++i) {
        cin >> altitude[i];
    }

    vector<int64> diff(N + 1);
    for (int i = 1; i <= N; ++i) {
        diff[i] = altitude[i] - altitude[i - 1];
    }

    auto contribution = [&](int64 d) -> int64 {
        if (d > 0) {
            return -S * d;
        }
        return -T * d;
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