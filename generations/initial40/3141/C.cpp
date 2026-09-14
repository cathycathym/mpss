#include <iostream>
#include <vector>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, Q;
    long long S, T;
    cin >> N >> Q >> S >> T;

    vector<long long> difference(N + 1);
    long long previousAltitude, currentAltitude;
    cin >> previousAltitude;  // A_0

    for (int i = 1; i <= N; ++i) {
        cin >> currentAltitude;
        difference[i] = currentAltitude - previousAltitude;
        previousAltitude = currentAltitude;
    }

    auto contribution = [&](long long d) -> long long {
        if (d > 0) {
            return -S * d;
        }
        return -T * d;
    };

    long long temperature = 0;
    for (int i = 1; i <= N; ++i) {
        temperature += contribution(difference[i]);
    }

    while (Q--) {
        int L, R;
        long long X;
        cin >> L >> R >> X;

        temperature -= contribution(difference[L]);
        difference[L] += X;
        temperature += contribution(difference[L]);

        if (R < N) {
            temperature -= contribution(difference[R + 1]);
            difference[R + 1] -= X;
            temperature += contribution(difference[R + 1]);
        }

        cout << temperature << '\n';
    }

    return 0;
}