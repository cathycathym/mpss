#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<vector<int>> mode(n, vector<int>(n));
    for (int left = 0; left < n; ++left) {
        for (int right = left; right < n; ++right) {
            cin >> mode[left][right];
        }
    }

    vector<int> color(n, -1);
    vector<int> total(3, 0);

    // Character names are initially interchangeable.
    color[0] = 0;
    total[0] = 1;

    for (int right = 1; right < n; ++right) {
        vector<int> possible;

        for (int candidate = 0; candidate < 3; ++candidate) {
            int count[3] = {0, 0, 0};
            ++count[candidate];

            bool valid = (mode[right][right] == 1);

            for (int left = right - 1; left >= 0 && valid; --left) {
                ++count[color[left]];
                int predicted = max(count[0], max(count[1], count[2]));

                if (predicted != mode[left][right]) {
                    valid = false;
                }
            }

            if (valid) {
                possible.push_back(candidate);
            }
        }

        // If several characters are indistinguishable from the hints so far,
        // balancing their total frequencies preserves the uniquely recoverable
        // globally most frequent character.
        int chosen = possible[0];
        for (int candidate : possible) {
            if (total[candidate] < total[chosen]) {
                chosen = candidate;
            }
        }

        color[right] = chosen;
        ++total[chosen];
    }

    int mostCommon = 0;
    if (total[1] > total[mostCommon]) mostCommon = 1;
    if (total[2] > total[mostCommon]) mostCommon = 2;

    bool first = true;
    for (int position = 0; position < n; ++position) {
        if (color[position] == mostCommon) {
            if (!first) cout << ' ';
            cout << position + 1;
            first = false;
        }
    }
    cout << '\n';

    return 0;
}
