#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <algorithm>

using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int W, H, K, L;
    cin >> W >> H >> K >> L;

    int xh, yh, xv, yv;
    cin >> xh >> yh >> xv >> yv;

    vector<string> grid(H);
    for (int r = 0; r < H; ++r) {
        cin >> grid[r];
    }

    const int N = H * W;
    vector<int> left(N, 0), right(N, 0);
    vector<int> up(N, 0), down(N, 0);

    auto id = [W](int r, int c) {
        return r * W + c;
    };

    for (int r = 0; r < H; ++r) {
        for (int c = 0; c < W; ++c) {
            if (grid[r][c] == 'X') continue;

            int cur = id(r, c);
            left[cur] = 1;
            up[cur] = 1;

            if (c > 0) left[cur] += left[id(r, c - 1)];
            if (r > 0) up[cur] += up[id(r - 1, c)];
        }
    }

    for (int r = H - 1; r >= 0; --r) {
        for (int c = W - 1; c >= 0; --c) {
            if (grid[r][c] == 'X') continue;

            int cur = id(r, c);
            right[cur] = 1;
            down[cur] = 1;

            if (c + 1 < W) right[cur] += right[id(r, c + 1)];
            if (r + 1 < H) down[cur] += down[id(r + 1, c)];
        }
    }

    const int start = id(yh, xv);
    vector<char> visited(N, false);
    queue<int> bfs;

    visited[start] = true;
    bfs.push(start);

    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};

    while (!bfs.empty()) {
        int cur = bfs.front();
        bfs.pop();

        int r = cur / W;
        int c = cur % W;

        if (grid[r][c] == '*') {
            cout << "YES\n";
            return 0;
        }

        for (int direction = 0; direction < 4; ++direction) {
            int nr = r + dr[direction];
            int nc = c + dc[direction];

            if (nr < 0 || nr >= H || nc < 0 || nc >= W) continue;
            if (grid[nr][nc] == 'X') continue;

            int next = id(nr, nc);
            if (visited[next]) continue;

            bool possible;

            if (nr != r) {
                int commonHorizontal =
                    min(left[cur], left[next]) +
                    min(right[cur], right[next]) - 1;
                possible = (commonHorizontal >= K);
            } else {
                int commonVertical =
                    min(up[cur], up[next]) +
                    min(down[cur], down[next]) - 1;
                possible = (commonVertical >= L);
            }

            if (possible) {
                visited[next] = true;
                bfs.push(next);
            }
        }
    }

    cout << "NO\n";
    return 0;
}