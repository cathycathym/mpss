#include <iostream>
#include <vector>
#include <string>
#include <deque>
#include <cstdint>
#include <algorithm>
#include <limits>

using namespace std;

class SpatialSet {
private:
    int rows, cols;
    vector<uint8_t> state;
    vector<int>& dist;
    deque<int>& bfsQueue;
    static constexpr int INF = numeric_limits<int>::max() / 4;

    void split(int r1, int r2, int c1, int c2,
               int& lr1, int& lr2, int& lc1, int& lc2,
               int& rr1, int& rr2, int& rc1, int& rc2,
               int& leftArea) const {
        int height = r2 - r1 + 1;
        int width = c2 - c1 + 1;

        if (height > width) {
            int mid = (r1 + r2) / 2;
            lr1 = r1;
            lr2 = mid;
            lc1 = c1;
            lc2 = c2;

            rr1 = mid + 1;
            rr2 = r2;
            rc1 = c1;
            rc2 = c2;

            leftArea = (mid - r1 + 1) * width;
        } else {
            int mid = (c1 + c2) / 2;
            lr1 = r1;
            lr2 = r2;
            lc1 = c1;
            lc2 = mid;

            rr1 = r1;
            rr2 = r2;
            rc1 = mid + 1;
            rc2 = c2;

            leftArea = height * (mid - c1 + 1);
        }
    }

    void drain(int node, int r1, int r2, int c1, int c2, int newDistance) {
        if (state[node] == 2) return;

        if (r1 == r2 && c1 == c2) {
            int cell = r1 * cols + c1;
            if (dist[cell] == INF) {
                dist[cell] = newDistance;
                bfsQueue.push_back(cell);
            }
            state[node] = 2;
            return;
        }

        int lr1, lr2, lc1, lc2;
        int rr1, rr2, rc1, rc2;
        int leftArea;
        split(r1, r2, c1, c2,
              lr1, lr2, lc1, lc2,
              rr1, rr2, rc1, rc2, leftArea);

        int leftNode = node + 1;
        int rightNode = node + 2 * leftArea;

        drain(leftNode, lr1, lr2, lc1, lc2, newDistance);
        drain(rightNode, rr1, rr2, rc1, rc2, newDistance);
        state[node] = 2;
    }

    void extract(int node, int r1, int r2, int c1, int c2,
                 int qr1, int qr2, int qc1, int qc2, int newDistance) {
        if (state[node] == 2) return;
        if (r2 < qr1 || qr2 < r1 || c2 < qc1 || qc2 < c1) return;

        if (qr1 <= r1 && r2 <= qr2 && qc1 <= c1 && c2 <= qc2) {
            drain(node, r1, r2, c1, c2, newDistance);
            return;
        }

        if (r1 == r2 && c1 == c2) {
            int cell = r1 * cols + c1;
            if (dist[cell] == INF) {
                dist[cell] = newDistance;
                bfsQueue.push_back(cell);
            }
            state[node] = 2;
            return;
        }

        int lr1, lr2, lc1, lc2;
        int rr1, rr2, rc1, rc2;
        int leftArea;
        split(r1, r2, c1, c2,
              lr1, lr2, lc1, lc2,
              rr1, rr2, rc1, rc2, leftArea);

        int leftNode = node + 1;
        int rightNode = node + 2 * leftArea;

        extract(leftNode, lr1, lr2, lc1, lc2,
                qr1, qr2, qc1, qc2, newDistance);
        extract(rightNode, rr1, rr2, rc1, rc2,
                qr1, qr2, qc1, qc2, newDistance);

        state[node] =
            (state[leftNode] == 2 && state[rightNode] == 2) ? 2 : 1;
    }

public:
    SpatialSet(int r, int c, vector<int>& distances, deque<int>& queue)
        : rows(r), cols(c),
          state(static_cast<size_t>(2LL * r * c - 1), 0),
          dist(distances), bfsQueue(queue) {}

    void extractRectangle(int r1, int r2, int c1, int c2, int newDistance) {
        r1 = max(r1, 0);
        r2 = min(r2, rows - 1);
        c1 = max(c1, 0);
        c2 = min(c2, cols - 1);

        if (r1 > r2 || c1 > c2) return;

        extract(0, 0, rows - 1, 0, cols - 1,
                r1, r2, c1, c2, newDistance);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int R, C, N;
    cin >> R >> C >> N;

    int Sr, Sc, Gr, Gc;
    cin >> Sr >> Sc;
    cin >> Gr >> Gc;
    --Sr;
    --Sc;
    --Gr;
    --Gc;

    vector<string> grid(R);
    for (string& row : grid) cin >> row;

    const int total = R * C;
    const int start = Sr * C + Sc;
    const int goal = Gr * C + Gc;
    const int INF = numeric_limits<int>::max() / 4;

    vector<int> dist(total, INF);
    vector<uint8_t> finalized(total, 0);
    deque<int> q;

    dist[start] = 0;
    q.push_back(start);

    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};

    if (N == 1) {
        while (!q.empty()) {
            int v = q.front();
            q.pop_front();

            if (finalized[v]) continue;
            finalized[v] = 1;

            if (v == goal) {
                cout << dist[v] << '\n';
                return 0;
            }

            int r = v / C;
            int c = v % C;

            for (int direction = 0; direction < 4; ++direction) {
                int nr = r + dr[direction];
                int nc = c + dc[direction];

                if (nr < 0 || nr >= R || nc < 0 || nc >= C) continue;

                int to = nr * C + nc;
                int cost = (grid[nr][nc] == '#') ? 1 : 0;

                if (dist[to] > dist[v] + cost) {
                    dist[to] = dist[v] + cost;
                    if (cost == 0) q.push_front(to);
                    else q.push_back(to);
                }
            }
        }
    } else {
        SpatialSet unvisited(R, C, dist, q);

        while (!q.empty()) {
            int v = q.front();
            q.pop_front();

            if (finalized[v]) continue;
            finalized[v] = 1;

            if (v == goal) {
                cout << dist[v] << '\n';
                return 0;
            }

            int r = v / C;
            int c = v % C;
            int currentDistance = dist[v];

            for (int direction = 0; direction < 4; ++direction) {
                int nr = r + dr[direction];
                int nc = c + dc[direction];

                if (nr < 0 || nr >= R || nc < 0 || nc >= C) continue;
                if (grid[nr][nc] != '.') continue;

                int to = nr * C + nc;
                if (dist[to] > currentDistance) {
                    dist[to] = currentDistance;
                    q.push_front(to);
                }
            }

            unvisited.extractRectangle(
                r - (N - 1), r + (N - 1),
                c - N, c + N,
                currentDistance + 1
            );

            unvisited.extractRectangle(
                r - N, r + N,
                c - (N - 1), c + (N - 1),
                currentDistance + 1
            );
        }
    }

    cout << dist[goal] << '\n';
    return 0;
}
