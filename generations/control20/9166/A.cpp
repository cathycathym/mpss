#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M;
    cin >> N >> M;

    vector<long long> sprinklers(N), flowers(M);
    for (long long &x : sprinklers) cin >> x;
    for (long long &x : flowers) cin >> x;

    auto feasible = [&](long long K, string *configuration) -> bool {
        int flowerIndex = 0;
        int deadline = -1;
        int lastRight = -1;

        string directions;
        if (configuration != nullptr) {
            directions.assign(N, 'R');
        }

        // Flowers strictly to the left of the first sprinkler must be
        // covered by a left-facing sprinkler.
        if (flowerIndex < M && flowers[flowerIndex] < sprinklers[0]) {
            long long firstFlower = flowers[flowerIndex];
            int lastCandidate = int(upper_bound(
                sprinklers.begin(), sprinklers.end(), firstFlower + K
            ) - sprinklers.begin());

            if (lastCandidate == 0) return false;
            deadline = lastCandidate;

            while (flowerIndex < M &&
                   flowers[flowerIndex] < sprinklers[0]) {
                ++flowerIndex;
            }
        }

        for (int i = 0; i < N; ++i) {
            int oneBasedIndex = i + 1;

            if (deadline == oneBasedIndex) {
                if (configuration != nullptr) directions[i] = 'L';
                deadline = -1;
            } else {
                lastRight = i;
                if (configuration != nullptr) directions[i] = 'R';
            }

            // Flowers exactly at a sprinkler position are always watered.
            while (flowerIndex < M &&
                   flowers[flowerIndex] == sprinklers[i]) {
                ++flowerIndex;
            }

            if (i + 1 == N) {
                // No future left-facing sprinkler can cover these flowers.
                if (flowerIndex < M) {
                    if (lastRight == -1 ||
                        flowers.back() > sprinklers[lastRight] + K) {
                        return false;
                    }
                    flowerIndex = M;
                }
                continue;
            }

            long long nextPosition = sprinklers[i + 1];
            long long rightReach =
                (lastRight == -1 ? -1LL : sprinklers[lastRight] + K);

            long long firstUncovered = -1;

            // Process all flowers in the open gap between the current
            // sprinkler position and the next one.
            while (flowerIndex < M &&
                   flowers[flowerIndex] < nextPosition) {
                if (firstUncovered == -1 &&
                    flowers[flowerIndex] > rightReach) {
                    firstUncovered = flowers[flowerIndex];
                }
                ++flowerIndex;
            }

            if (firstUncovered != -1) {
                int lastCandidate = int(upper_bound(
                    sprinklers.begin(), sprinklers.end(),
                    firstUncovered + K
                ) - sprinklers.begin());

                // No sprinkler to the right can reach this flower.
                if (lastCandidate <= oneBasedIndex) return false;

                if (deadline == -1 || lastCandidate < deadline) {
                    deadline = lastCandidate;
                }
            }
        }

        if (deadline != -1 || flowerIndex != M) return false;

        if (configuration != nullptr) {
            *configuration = directions;
        }
        return true;
    };

    const long long MAX_K = 1000000000LL;

    if (!feasible(MAX_K, nullptr)) {
        cout << -1 << '\n';
        return 0;
    }

    long long low = 0;
    long long high = MAX_K;

    while (low < high) {
        long long middle = low + (high - low) / 2;
        if (feasible(middle, nullptr)) {
            high = middle;
        } else {
            low = middle + 1;
        }
    }

    string answer;
    feasible(low, &answer);

    cout << low << '\n';
    cout << answer << '\n';
    return 0;
}
