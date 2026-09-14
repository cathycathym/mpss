#include <bits/stdc++.h>
using namespace std;

int n, globalMode;
vector<unsigned short> hintValue;
vector<array<int, 3>> prefixCount;
vector<int> acronym;
long long exploredNodes = 0;

inline int hint(int left, int right) {
    return hintValue[left * n + right];
}

bool validEndingAt(int position) {
    for (int left = 0; left <= position; ++left) {
        int mostCommon = 0;
        for (int color = 0; color < 3; ++color) {
            mostCommon = max(
                mostCommon,
                prefixCount[position + 1][color] - prefixCount[left][color]
            );
        }
        if (mostCommon != hint(left, position)) {
            return false;
        }
    }
    return true;
}

bool globallyFeasible(int assignedLength) {
    int remaining = n - assignedLength;
    const auto &used = prefixCount[assignedLength];

    for (int winner = 0; winner < 3; ++winner) {
        if (used[winner] > globalMode) {
            continue;
        }

        int neededForWinner = globalMode - used[winner];
        if (neededForWinner > remaining) {
            continue;
        }

        bool possible = true;
        int otherCapacity = 0;

        for (int color = 0; color < 3; ++color) {
            if (color == winner) {
                continue;
            }
            if (used[color] >= globalMode) {
                possible = false;
                break;
            }
            otherCapacity += globalMode - 1 - used[color];
        }

        if (!possible) {
            continue;
        }

        int positionsForOthers = remaining - neededForWinner;
        if (positionsForOthers <= otherCapacity) {
            return true;
        }
    }

    return false;
}

vector<int> validChoicesAt(int position, int maximumUsedColor) {
    vector<int> choices;
    int largestChoice = min(2, maximumUsedColor + 1);

    for (int color = 0; color <= largestChoice; ++color) {
        acronym[position] = color;
        prefixCount[position + 1] = prefixCount[position];
        ++prefixCount[position + 1][color];

        if (validEndingAt(position) && globallyFeasible(position + 1)) {
            choices.push_back(color);
        }
    }

    return choices;
}

bool solve(int position, int maximumUsedColor) {
    ++exploredNodes;

    if (position == n) {
        const auto &totals = prefixCount[n];
        int winner = max_element(totals.begin(), totals.end()) - totals.begin();

        for (int color = 0; color < 3; ++color) {
            if (color != winner && totals[color] >= totals[winner]) {
                return false;
            }
        }
        return totals[winner] == globalMode;
    }

    vector<int> choices = validChoicesAt(position, maximumUsedColor);

    if (position + 1 < n && choices.size() > 1) {
        stable_sort(choices.begin(), choices.end(), [&](int first, int second) {
            auto countNextChoices = [&](int color) {
                acronym[position] = color;
                prefixCount[position + 1] = prefixCount[position];
                ++prefixCount[position + 1][color];

                int nextMaximum = max(maximumUsedColor, color);
                return static_cast<int>(
                    validChoicesAt(position + 1, nextMaximum).size()
                );
            };

            return countNextChoices(first) > countNextChoices(second);
        });
    }

    for (int color : choices) {
        acronym[position] = color;
        prefixCount[position + 1] = prefixCount[position];
        ++prefixCount[position + 1][color];

        if (solve(position + 1, max(maximumUsedColor, color))) {
            return true;
        }
    }

    return false;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n;
    hintValue.assign(n * n, 0);

    for (int left = 0; left < n; ++left) {
        for (int right = left; right < n; ++right) {
            int value;
            cin >> value;
            hintValue[left * n + right] =
                static_cast<unsigned short>(value);
        }
    }

    globalMode = hint(0, n - 1);
    acronym.assign(n, 0);
    prefixCount.assign(n + 1, {0, 0, 0});

    acronym[0] = 0;
    prefixCount[1] = {1, 0, 0};

    bool found = validEndingAt(0) && globallyFeasible(1) && solve(1, 0);
    if (!found) {
        return 0;
    }

    const auto &totals = prefixCount[n];
    int bColor = max_element(totals.begin(), totals.end()) - totals.begin();

    bool firstOutput = true;
    for (int position = 0; position < n; ++position) {
        if (acronym[position] == bColor) {
            if (!firstOutput) {
                cout << ' ';
            }
            cout << position + 1;
            firstOutput = false;
        }
    }
    cout << '\n';

    return 0;
}
