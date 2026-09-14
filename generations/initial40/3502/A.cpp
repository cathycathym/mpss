#include <iostream>
#include <vector>
using namespace std;

using int64 = long long;

vector<int> buildLowerHull(const vector<int64>& value) {
    vector<int> hull;

    for (int i = 0; i < static_cast<int>(value.size()); ++i) {
        while (hull.size() >= 2) {
            int x = hull[hull.size() - 2];
            int y = hull[hull.size() - 1];

            // slope(x, y) >= slope(y, i)
            int64 left = (value[y] - value[x]) * (i - y);
            int64 right = (value[i] - value[y]) * (y - x);

            if (left >= right) {
                hull.pop_back();
            } else {
                break;
            }
        }
        hull.push_back(i);
    }

    return hull;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int H, W;
    cin >> H >> W;

    vector<int64> A(H), B(W);
    for (int i = 0; i < H; ++i) cin >> A[i];
    for (int j = 0; j < W; ++j) cin >> B[j];

    vector<int> rows = buildLowerHull(A);
    vector<int> cols = buildLowerHull(B);

    int ri = 0;
    int ci = 0;
    int64 answer = 0;

    while (ri + 1 < static_cast<int>(rows.size()) ||
           ci + 1 < static_cast<int>(cols.size())) {

        if (ri + 1 == static_cast<int>(rows.size())) {
            int width = cols[ci + 1] - cols[ci];
            answer += static_cast<int64>(width) * A[rows[ri]];
            ++ci;
        } else if (ci + 1 == static_cast<int>(cols.size())) {
            int height = rows[ri + 1] - rows[ri];
            answer += static_cast<int64>(height) * B[cols[ci]];
            ++ri;
        } else {
            int height = rows[ri + 1] - rows[ri];
            int width = cols[ci + 1] - cols[ci];

            int64 deltaA = A[rows[ri + 1]] - A[rows[ri]];
            int64 deltaB = B[cols[ci + 1]] - B[cols[ci]];

            // Compare deltaA / height <= deltaB / width.
            if (deltaA * width <= deltaB * height) {
                answer += static_cast<int64>(height) * B[cols[ci]];
                ++ri;
            } else {
                answer += static_cast<int64>(width) * A[rows[ri]];
                ++ci;
            }
        }
    }

    cout << answer << '\n';
    return 0;
}