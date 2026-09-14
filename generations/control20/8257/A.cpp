#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
using namespace std;

using int64 = long long;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, L;
    cin >> N >> L;

    vector<int> X(N);
    vector<int64> countAt(L + 1, 0), sumAt(L + 1, 0);

    int A = L, B = 0;
    int64 totalSum = 0;

    for (int &x : X) {
        cin >> x;
        A = min(A, x);
        B = max(B, x);
        ++countAt[x];
        sumAt[x] += x;
        totalSum += x;
    }

    // Prefix information lets us split the balls around each finishing
    // position in O(1) time.
    vector<int64> prefixCount(L + 1), prefixSum(L + 1);
    for (int x = 0; x <= L; ++x) {
        prefixCount[x] = countAt[x] + (x == 0 ? 0 : prefixCount[x - 1]);
        prefixSum[x] = sumAt[x] + (x == 0 ? 0 : prefixSum[x - 1]);
    }

    auto carryCostLeftThenRight = [&](int G) -> int64 {
        // Canonical route: S -> A -> B -> G.
        //
        // A ball is optimally collected on its last visit.
        if (G <= A) {
            return totalSum - int64(N) * G;
        }
        if (G >= B) {
            return int64(N) * G - totalSum;
        }

        // Balls x < G are last visited while going A -> B:
        // remaining distance = (B-x) + (B-G).
        // Balls x >= G are last visited while going B -> G:
        // remaining distance = x-G.
        int64 leftCount = prefixCount[G - 1];
        int64 leftSum = prefixSum[G - 1];
        int64 rightCount = N - leftCount;
        int64 rightSum = totalSum - leftSum;

        return leftCount * (int64(2) * B - G) - leftSum
             + rightSum - rightCount * G;
    };

    auto carryCostRightThenLeft = [&](int G) -> int64 {
        // Canonical route: S -> B -> A -> G.
        if (G <= A) {
            return totalSum - int64(N) * G;
        }
        if (G >= B) {
            return int64(N) * G - totalSum;
        }

        // Balls x <= G are last visited while going A -> G:
        // remaining distance = G-x.
        // Balls x > G are last visited while going B -> A:
        // remaining distance = (x-A) + (G-A).
        int64 leftCount = prefixCount[G];
        int64 leftSum = prefixSum[G];
        int64 rightCount = N - leftCount;
        int64 rightSum = totalSum - leftSum;

        return leftCount * G - leftSum
             + rightSum + rightCount * (int64(G) - int64(2) * A);
    };

    int Q;
    cin >> Q;

    while (Q--) {
        int S, G;
        int64 T;
        cin >> S >> G >> T;

        // In any route, compare the last visits to the leftmost and
        // rightmost ball positions. Their order gives one of these two
        // lower bounds, and the corresponding canonical route attains it.
        int64 routeLeftThenRight =
            llabs(int64(S) - A) + (B - A) + llabs(int64(B) - G);

        int64 routeRightThenLeft =
            llabs(int64(S) - B) + (B - A) + llabs(int64(A) - G);

        int64 timeLeftThenRight =
            N + routeLeftThenRight + carryCostLeftThenRight(G);

        int64 timeRightThenLeft =
            N + routeRightThenLeft + carryCostRightThenLeft(G);

        int64 minimumTime = min(timeLeftThenRight, timeRightThenLeft);
        cout << (minimumTime <= T ? "Yes\n" : "No\n");
    }

    return 0;
}
