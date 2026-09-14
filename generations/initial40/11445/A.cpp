#include <iostream>
#include <vector>
#include <string>
#include <bitset>
using namespace std;

static const int MAX_C = 500;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int R, C, M;
    cin >> R >> C >> M;

    vector<bitset<MAX_C>> water(R);
    for (int r = 0; r < R; ++r) {
        string row;
        cin >> row;
        for (int c = 0; c < C; ++c) {
            if (row[c] == '.') {
                water[r].set(c);
            }
        }
    }

    string signals;
    cin >> signals;

    // Before any signals, Nautilus may be in any water cell.
    vector<bitset<MAX_C>> possible = water;
    vector<bitset<MAX_C>> nextPossible(R);

    for (char signal : signals) {
        for (int r = 0; r < R; ++r) {
            bitset<MAX_C> reached;

            if (signal == 'E' || signal == '?') {
                reached |= (possible[r] << 1);
            }
            if (signal == 'W' || signal == '?') {
                reached |= (possible[r] >> 1);
            }
            if ((signal == 'S' || signal == '?') && r > 0) {
                reached |= possible[r - 1];
            }
            if ((signal == 'N' || signal == '?') && r + 1 < R) {
                reached |= possible[r + 1];
            }

            nextPossible[r] = reached & water[r];
        }

        possible.swap(nextPossible);
    }

    int answer = 0;
    for (int r = 0; r < R; ++r) {
        answer += static_cast<int>(possible[r].count());
    }

    cout << answer << '\n';
    return 0;
}