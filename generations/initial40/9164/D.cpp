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
    int target = -1;

    for (int y = 0; y < H; ++y) {
        cin >> grid[y];
        for (int x = 0; x < W; ++x) {
            if (grid[y][x] == '*') {
                target = y * W + x;
            }
        }
    }

    const int N = W * H;
    vector<uint16_t> left(N), right(N), up(N), down(N);

    auto blocked = [&](int x, int y) {
        return grid[y][x] == 'x' || grid[y][x] == 'X';
    };

    for (int y = 0; y < H; ++y) {
        int run = 0;
        for (int x = 0; x < W; ++x) {
            int id = y * W + x;
            if (blocked(x, y)) {
                run = 0;
            } else {
                left[id] = static_cast<uint16_t>(++run);
            }
        }

        run = 0;
        for (int x = W - 1; x >= 0; --x) {
            int id = y * W + x;
            if (blocked(x, y)) {
                run = 0;
            } else {
                right[id] = static_cast<uint16_t>(++run);
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
                up[id] = static_cast<uint16_t>(++run);
            }
        }

        run = 0;
        for (int y = H - 1; y >= 0; --y) {
            int id = y * W + x;
            if (blocked(x, y)) {
                run = 0;
            } else {
                down[id] = static_cast<uint16_t>(++run);
            }
        }
    }

    vector<uint8_t> valid(N, 0);
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            if (blocked(x, y)) continue;

            int id = y * W + x;
            int horizontalSpace = int(left[id]) + int(right[id]) - 1;
            int verticalSpace = int(up[id]) + int(down[id]) - 1;

            if (horizontalSpace >= K && verticalSpace >= L) {
                valid[id] = 1;
            }
        }
    }

    const int start = yh * W + xv;

    vector<uint8_t> visited(N, 0);
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

        auto tryHorizontalMove = [&](int next) {
            if (visited[next] || !valid[next]) return;

            int commonVertical =
                min<int>(up[current], up[next]) +
                min<int>(down[current], down[next]) - 1;

            if (commonVertical >= L) {
                visited[next] = 1;
                queue.push_back(next);
            }
        };

        auto tryVerticalMove = [&](int next) {
            if (visited[next] || !valid[next]) return;

            int commonHorizontal =
                min<int>(left[current], left[next]) +
                min<int>(right[current], right[next]) - 1;

            if (commonHorizontal >= K) {
                visited[next] = 1;
                queue.push_back(next);
            }
        };

        if (x > 0) tryHorizontalMove(current - 1);
        if (x + 1 < W) tryHorizontalMove(current + 1);
        if (y > 0) tryVerticalMove(current - W);
        if (y + 1 < H) tryVerticalMove(current + W);
    }

    cout << "NO\n";
    return 0;
}