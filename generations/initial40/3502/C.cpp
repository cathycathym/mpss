#include <iostream>
#include <vector>
using namespace std;

using int64 = long long;

// Builds the lower convex hull of points (i, value[i]).
// Consecutive hull slopes are strictly increasing.
vector<int> buildHull(const vector<int64>& value) {
    vector<int> hull;

    for (int i = 0; i < static_cast<int>(value.size()); ++i) {
        while (hull.size() >= 2) {
            int p = hull[hull.size() - 2];
            int q = hull[hull.size() - 1];

            // slope(p,q) >= slope(q,i)
            int64 left =
                (value[q] - value[p]) * static_cast<int64>(i - q);
            int64 right =
                (value[i] - value[q]) * static_cast<int64>(q - p);

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

    vector<int> hullA = buildHull(A);
    vector<int> hullB = buildHull(B);

    int x = 0;
    int y = 0;
    int64 answer = 0;

    while (x + 1 < static_cast<int>(hullA.size()) ||
           y + 1 < static_cast<int>(hullB.size())) {

        if (x + 1 == static_cast<int>(hullA.size())) {
            // Only rightward movement remains.
            int columns = hullB[y + 1] - hullB[y];
            answer += static_cast<int64>(columns) * A[hullA[x]];
            ++y;
        } else if (y + 1 == static_cast<int>(hullB.size())) {
            // Only downward movement remains.
            int rows = hullA[x + 1] - hullA[x];
            answer += static_cast<int64>(rows) * B[hullB[y]];
            ++x;
        } else {
            int rows = hullA[x + 1] - hullA[x];
            int columns = hullB[y + 1] - hullB[y];

            int64 deltaA = A[hullA[x + 1]] - A[hullA[x]];
            int64 deltaB = B[hullB[y + 1]] - B[hullB[y]];

            // deltaA / rows <= deltaB / columns
            if (deltaA * static_cast<int64>(columns) <=
                deltaB * static_cast<int64>(rows)) {
                answer += static_cast<int64>(rows) * B[hullB[y]];
                ++x;
            } else {
                answer += static_cast<int64>(columns) * A[hullA[x]];
                ++y;
            }
        }
    }

    cout << answer << '\n';
    return 0;
}