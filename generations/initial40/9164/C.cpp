#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int W, H, K, L;
    cin >> W >> H >> K >> L;

    int xh, yh, xv, yv;
    cin >> xh >> yh >> xv >> yv;

    vector<string> grid(H);
    int targetX = -1, targetY = -1;

    for (int y = 0; y < H; ++y) {
        cin >> grid[y];
        for (int x = 0; x < W; ++x) {
            if (grid[y][x] == '*') {
                targetX = x;
                targetY = y;
            }
        }
    }

    const int N = W * H;
    vector<uint16_t> leftLen(N), rightLen(N);
    vector<uint16_t> upLen(N), downLen(N);

    auto blocked = [&](int x, int y) {
        return grid[y][x] == 'X' || grid[y][x] == 'x';
    };

    for (int y = 0; y < H; ++y) {
        int run = 0;
        for (int x = 0; x < W; ++x) {
            int id = y * W + x;
            if (blocked(x, y)) {
                run = 0;
            } else {
                ++run;
                leftLen[id] = static_cast<uint16_t>(run);
            }
        }

        run = 0;
        for (int x = W - 1; x >= 0; --x) {
            int id = y * W + x;
            if (blocked(x, y)) {
                run = 0;
            } else {
                ++run;
                rightLen[id] = static_cast<uint16_t>(run);
            }
        }
    }

    for (int x = 0; x < W; ++x) {
        int run = 0;
        for (int y = 0; y < H; ++y) {
            int id = y * W + x;
            if (blocked(x, y)) {
                run = 0;
            } else {
                ++run;
                upLen[id] = static_cast<uint16_t>(run);
            }
        }

        run = 0;
        for (int y = H - 1; y >= 0; --y) {
            int id = y * W + x;
            if (blocked(x, y)) {
                run = 0;
            } else {
                ++run;
                downLen[id] = static_cast<uint16_t>(run);
            }
        }
    }

    vector<unsigned char> valid(N, 0);
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            if (blocked(x, y)) continue;

            int id = y * W + x;
            int horizontalSpace =
                static_cast<int>(leftLen[id]) + rightLen[id] - 1;
            int verticalSpace =
                static_cast<int>(upLen[id]) + downLen[id] - 1;

            if (horizontalSpace >= K && verticalSpace >= L) {
                valid[id] = 1;
            }
        }
    }

    const int start = yh * W + xv;
    const int target = targetY * W + targetX;

    vector<unsigned char> visited(N, 0);
    vector<int> queue;
    queue.reserve(N);

    visited[start] = 1;
    queue.push_back(start);

    for (size_t head = 0; head < queue.size(); ++head) {
        int current = queue[head];

        if (current == target) {
            cout << "YES\n";
            return 0;
        }

        int x = current % W;
        int y = current / W;

        if (x > 0) {
            int next = current - 1;
            if (!visited[next] && valid[next]) {
                int commonVertical =
                    min<int>(upLen[current], upLen[next]) +
                    min<int>(downLen[current], downLen[next]) - 1;

                if (commonVertical >= L) {
                    visited[next] = 1;
                    queue.push_back(next);
                }
            }
        }

        if (x + 1 < W) {
            int next = current + 1;
            if (!visited[next] && valid[next]) {
                int commonVertical =
                    min<int>(upLen[current], upLen[next]) +
                    min<int>(downLen[current], downLen[next]) - 1;

                if (commonVertical >= L) {
                    visited[next] = 1;
                    queue.push_back(next);
                }
            }
        }

        if (y > 0) {
            int next = current - W;
            if (!visited[next] && valid[next]) {
                int commonHorizontal =
                    min<int>(leftLen[current], leftLen[next]) +
                    min<int>(rightLen[current], rightLen[next]) - 1;

                if (commonHorizontal >= K) {
                    visited[next] = 1;
                    queue.push_back(next);
                }
            }
        }

        if (y + 1 < H) {
            int next = current + W;
            if (!visited[next] && valid[next]) {
                int commonHorizontal =
                    min<int>(leftLen[current], leftLen[next]) +
                    min<int>(rightLen[current], rightLen[next]) - 1;

                if (commonHorizontal >= K) {
                    visited[next] = 1;
                    queue.push_back(next);
                }
            }
        }
    }

    cout << "NO\n";
    return 0;
}