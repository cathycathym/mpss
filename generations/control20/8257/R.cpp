#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <cstdlib>
using namespace std;

using int64 = long long;

struct Query {
    int s, g, t;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, L;
    cin >> N >> L;

    vector<int> frequency(L + 1, 0);
    for (int i = 0; i < N; ++i) {
        int x;
        cin >> x;
        ++frequency[x];
    }

    vector<int> position;
    vector<int> multiplicity;
    for (int x = 0; x <= L; ++x) {
        if (frequency[x] > 0) {
            position.push_back(x);
            multiplicity.push_back(frequency[x]);
        }
    }

    int Q;
    cin >> Q;

    vector<Query> queries(Q);
    int maximumTime = 0;
    for (Query& query : queries) {
        cin >> query.s >> query.g >> query.t;
        maximumTime = max(maximumTime, query.t);
    }

    const int M = static_cast<int>(position.size());

    int64 distinctPositionLowerBound =
        static_cast<int64>(M) * (M - 1) / 2;

    if (static_cast<int64>(N) + distinctPositionLowerBound > maximumTime) {
        for (int i = 0; i < Q; ++i) {
            cout << "No\n";
        }
        return 0;
    }

    if (M == 1) {
        const int x = position[0];
        for (const Query& query : queries) {
            int64 minimumTime =
                static_cast<int64>(N) +
                static_cast<int64>(N + 1) * abs(query.g - x) +
                abs(query.s - x);

            cout << (minimumTime <= query.t ? "Yes\n" : "No\n");
        }
        return 0;
    }

    vector<int64> prefixBalls(M + 1, 0);
    for (int i = 0; i < M; ++i) {
        prefixBalls[i + 1] = prefixBalls[i] + multiplicity[i];
    }

    const int64 INF = numeric_limits<int64>::max() / 4;
    const size_t stateCount = static_cast<size_t>(M) * M;

    vector<int64> dpLeft(stateCount);
    vector<int64> dpRight(stateCount);
    vector<int64> finishAtWest(M);
    vector<int64> finishAtEast(M);

    auto index = [M](int left, int right) -> size_t {
        return static_cast<size_t>(left) * M + right;
    };

    auto calculate = [&](bool targetWest, vector<int64>& singletonCosts) {
        fill(dpLeft.begin(), dpLeft.end(), INF);
        fill(dpRight.begin(), dpRight.end(), INF);

        size_t full = index(0, M - 1);
        if (targetWest) {
            dpLeft[full] = 0;
        } else {
            dpRight[full] = 0;
        }

        for (int length = M - 1; length >= 1; --length) {
            for (int left = 0; left + length <= M; ++left) {
                int right = left + length - 1;
                size_t current = index(left, right);

                int64 visited =
                    prefixBalls[right + 1] - prefixBalls[left];
                int64 movementWeight =
                    static_cast<int64>(N) - visited + 1;

                int64 bestFromLeft = INF;
                int64 bestFromRight = INF;

                if (left > 0) {
                    size_t next = index(left - 1, right);

                    if (dpLeft[next] < INF) {
                        bestFromLeft = min(
                            bestFromLeft,
                            movementWeight *
                                    (position[left] - position[left - 1]) +
                                dpLeft[next]);

                        bestFromRight = min(
                            bestFromRight,
                            movementWeight *
                                    (position[right] - position[left - 1]) +
                                dpLeft[next]);
                    }
                }

                if (right + 1 < M) {
                    size_t next = index(left, right + 1);

                    if (dpRight[next] < INF) {
                        bestFromLeft = min(
                            bestFromLeft,
                            movementWeight *
                                    (position[right + 1] - position[left]) +
                                dpRight[next]);

                        bestFromRight = min(
                            bestFromRight,
                            movementWeight *
                                    (position[right + 1] - position[right]) +
                                dpRight[next]);
                    }
                }

                dpLeft[current] = bestFromLeft;
                dpRight[current] = bestFromRight;
            }
        }

        for (int i = 0; i < M; ++i) {
            size_t singleton = index(i, i);
            singletonCosts[i] =
                min(dpLeft[singleton], dpRight[singleton]);
        }
    };

    calculate(true, finishAtWest);
    calculate(false, finishAtEast);

    auto costFromFinishPosition =
        [&](int g, const vector<int64>& singletonCosts) -> int64 {
        auto it = lower_bound(position.begin(), position.end(), g);

        int64 answer = INF;

        if (it != position.end()) {
            int rightIndex = static_cast<int>(it - position.begin());
            answer = min(
                answer,
                static_cast<int64>(N + 1) *
                        (position[rightIndex] - g) +
                    singletonCosts[rightIndex]);
        }

        if (it != position.begin()) {
            int leftIndex = static_cast<int>(it - position.begin()) - 1;
            answer = min(
                answer,
                static_cast<int64>(N + 1) *
                        (g - position[leftIndex]) +
                    singletonCosts[leftIndex]);
        }

        return answer;
    };

    for (const Query& query : queries) {
        int64 viaWest =
            costFromFinishPosition(query.g, finishAtWest);
        int64 viaEast =
            costFromFinishPosition(query.g, finishAtEast);

        if (viaWest < INF) {
            viaWest += abs(query.s - position.front());
        }
        if (viaEast < INF) {
            viaEast += abs(query.s - position.back());
        }

        int64 minimumTime = static_cast<int64>(N) + min(viaWest, viaEast);
        cout << (minimumTime <= query.t ? "Yes\n" : "No\n");
    }

    return 0;
}
