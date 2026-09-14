#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int R, C, M;
    cin >> R >> C >> M;

    const int K = (C + 63) / 64;
    vector<vector<uint64_t>> water(R, vector<uint64_t>(K, 0));

    for (int r = 0; r < R; ++r) {
        string row;
        cin >> row;
        for (int c = 0; c < C; ++c) {
            if (row[c] == '.') {
                water[r][c / 64] |= uint64_t(1) << (c % 64);
            }
        }
    }

    string signals;
    cin >> signals;

    // Before any signals, every water cell is a possible starting position.
    vector<vector<uint64_t>> current = water;
    vector<vector<uint64_t>> next(R, vector<uint64_t>(K, 0));

    for (char signal : signals) {
        for (int r = 0; r < R; ++r) {
            for (int w = 0; w < K; ++w) {
                uint64_t destinations = 0;

                // A submarine ending in row r after moving north
                // must previously have been in row r + 1.
                if ((signal == 'N' || signal == '?') && r + 1 < R) {
                    destinations |= current[r + 1][w];
                }

                // A submarine ending in row r after moving south
                // must previously have been in row r - 1.
                if ((signal == 'S' || signal == '?') && r > 0) {
                    destinations |= current[r - 1][w];
                }

                // Moving east increases the column, which is a left shift.
                if (signal == 'E' || signal == '?') {
                    uint64_t shifted = current[r][w] << 1;
                    if (w > 0) {
                        shifted |= current[r][w - 1] >> 63;
                    }
                    destinations |= shifted;
                }

                // Moving west decreases the column, which is a right shift.
                if (signal == 'W' || signal == '?') {
                    uint64_t shifted = current[r][w] >> 1;
                    if (w + 1 < K) {
                        shifted |= current[r][w + 1] << 63;
                    }
                    destinations |= shifted;
                }

                next[r][w] = destinations & water[r][w];
            }
        }

        current.swap(next);
    }

    long long answer = 0;
    for (int r = 0; r < R; ++r) {
        for (int w = 0; w < K; ++w) {
            answer += __builtin_popcountll(current[r][w]);
        }
    }

    cout << answer << '\n';
    return 0;
}