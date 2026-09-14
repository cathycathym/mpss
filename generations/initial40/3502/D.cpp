#include <iostream>
#include <vector>
using namespace std;

using int64 = long long;
using int128 = __int128_t;

vector<int> buildLowerHull(const vector<int64>& values) {
    vector<int> hull;

    for (int i = 0; i < static_cast<int>(values.size()); ++i) {
        while (hull.size() >= 2) {
            int p = hull[hull.size() - 2];
            int q = hull[hull.size() - 1];

            int128 lhs = static_cast<int128>(values[q] - values[p]) * (i - q);
            int128 rhs = static_cast<int128>(values[i] - values[q]) * (q - p);

            if (lhs >= rhs) {
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
    for (int64& value : A) cin >> value;
    for (int64& value : B) cin >> value;

    vector<int> rows = buildLowerHull(A);
    vector<int> columns = buildLowerHull(B);

    int r = 0;
    int c = 0;
    int64 answer = 0;

    while (r + 1 < static_cast<int>(rows.size()) ||
           c + 1 < static_cast<int>(columns.size())) {
        if (r + 1 == static_cast<int>(rows.size())) {
            int width = columns[c + 1] - columns[c];
            answer += static_cast<int64>(width) * A[rows[r]];
            ++c;
        } else if (c + 1 == static_cast<int>(columns.size())) {
            int height = rows[r + 1] - rows[r];
            answer += static_cast<int64>(height) * B[columns[c]];
            ++r;
        } else {
            int height = rows[r + 1] - rows[r];
            int width = columns[c + 1] - columns[c];

            int64 deltaA = A[rows[r + 1]] - A[rows[r]];
            int64 deltaB = B[columns[c + 1]] - B[columns[c]];

            if (static_cast<int128>(deltaA) * width <=
                static_cast<int128>(deltaB) * height) {
                answer += static_cast<int64>(height) * B[columns[c]];
                ++r;
            } else {
                answer += static_cast<int64>(width) * A[rows[r]];
                ++c;
            }
        }
    }

    cout << answer << '\n';
    return 0;
}