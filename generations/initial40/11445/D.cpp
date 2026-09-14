#include <iostream>
#include <vector>
#include <string>
#include <bitset>
using namespace std;

constexpr int MAX_C = 500;

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
            if (row[c] == '.') water[r].set(c);
        }
    }

    string signals;
    cin >> signals;

    vector<bitset<MAX_C>> current = water;
    vector<bitset<MAX_C>> next(R);

    for (char signal : signals) {
        for (int r = 0; r < R; ++r) {
            bitset<MAX_C> reachable;

            if (signal == 'E' || signal == '?')
                reachable |= current[r] << 1;

            if (signal == 'W' || signal == '?')
                reachable |= current[r] >> 1;

            if ((signal == 'N' || signal == '?') && r + 1 < R)
                reachable |= current[r + 1];

            if ((signal == 'S' || signal == '?') && r > 0)
                reachable |= current[r - 1];

            next[r] = reachable & water[r];
        }

        current.swap(next);
    }

    int answer = 0;
    for (const auto& row : current)
        answer += static_cast<int>(row.count());

    cout << answer << '\n';
    return 0;
}