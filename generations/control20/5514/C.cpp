#include <iostream>
#include <vector>
#include <string>
#include <deque>
#include <cstdint>
#include <algorithm>
#include <limits>

using namespace std;

class UnvisitedGrid {
    int R, C, P, nodeCount;
    int levels, stride;
    vector<int> logicalBits;
    vector<int> words;
    vector<int> offset;
    vector<uint64_t> data;

    size_t at(int node, int level, int word) const {
        return static_cast<size_t>(node) * stride + offset[level] + word;
    }

    bool hasBit(int node, int col) const {
        return (data[at(node, 0, col >> 6)] >> (col & 63)) & 1ULL;
    }

    int nextSetBit(int node, int level, int pos) const {
        if (pos < 0) pos = 0;
        if (pos >= logicalBits[level]) return -1;

        int w = pos >> 6;
        int b = pos & 63;
        uint64_t x = data[at(node, level, w)] & (~0ULL << b);

        if (x != 0) {
            int result = (w << 6) + __builtin_ctzll(x);
            return result < logicalBits[level] ? result : -1;
        }

        if (level + 1 >= levels) return -1;

        int nextWord = nextSetBit(node, level + 1, w + 1);
        if (nextWord < 0 || nextWord >= words[level]) return -1;

        x = data[at(node, level, nextWord)];
        if (x == 0) return -1;

        int result = (nextWord << 6) + __builtin_ctzll(x);
        return result < logicalBits[level] ? result : -1;
    }

    void clearBitInNode(int node, int col) {
        int bitIndex = col;

        for (int level = 0; level < levels; ++level) {
            int w = bitIndex >> 6;
            uint64_t mask = 1ULL << (bitIndex & 63);
            uint64_t &value = data[at(node, level, w)];

            value &= ~mask;
            if (value != 0 || level + 1 == levels) break;

            // Word w has become empty, so clear its representative bit
            // in the next summary level.
            bitIndex = w;
        }
    }

    int findRow(int node, int col) const {
        while (node < P) {
            int left = node << 1;
            if (hasBit(left, col)) node = left;
            else node = left | 1;
        }
        return node - P;
    }

    template <class Callback>
    void extractRec(int node, int nl, int nr,
                    int ql, int qr, int cl, int cr,
                    Callback &callback) {
        if (qr < nl || nr < ql) return;

        int first = nextSetBit(node, 0, cl);
        if (first < 0 || first > cr) return;

        if (ql <= nl && nr <= qr) {
            int col = first;
            while (col >= 0 && col <= cr) {
                int row = findRow(node, col);
                erase(row, col);
                callback(row * C + col);
                col = nextSetBit(node, 0, cl);
            }
            return;
        }

        int mid = (nl + nr) >> 1;
        extractRec(node << 1, nl, mid,
                   ql, qr, cl, cr, callback);
        extractRec(node << 1 | 1, mid + 1, nr,
                   ql, qr, cl, cr, callback);
    }

public:
    UnvisitedGrid(int rows, int cols) : R(rows), C(cols) {
        P = 1;
        while (P < R) P <<= 1;
        nodeCount = 2 * P;

        logicalBits.clear();
        words.clear();
        offset.clear();

        int bits = C;
        stride = 0;
        while (true) {
            logicalBits.push_back(bits);
            offset.push_back(stride);
            int w = (bits + 63) >> 6;
            words.push_back(w);
            stride += w;
            if (w <= 1) break;
            bits = w;
        }
        levels = static_cast<int>(words.size());

        data.assign(static_cast<size_t>(nodeCount) * stride, 0);

        // Initialize valid leaves: every cell starts unvisited.
        for (int r = 0; r < R; ++r) {
            int node = P + r;
            for (int w = 0; w < words[0]; ++w) {
                data[at(node, 0, w)] = ~0ULL;
            }
            if ((C & 63) != 0) {
                data[at(node, 0, words[0] - 1)] =
                    (1ULL << (C & 63)) - 1ULL;
            }
        }

        // Build base bitsets of internal nodes.
        for (int node = P - 1; node >= 1; --node) {
            for (int w = 0; w < words[0]; ++w) {
                data[at(node, 0, w)] =
                    data[at(node << 1, 0, w)] |
                    data[at(node << 1 | 1, 0, w)];
            }
        }

        // Build summary bitsets.
        for (int node = 1; node < nodeCount; ++node) {
            for (int level = 1; level < levels; ++level) {
                int lowerWordCount = words[level - 1];
                for (int w = 0; w < lowerWordCount; ++w) {
                    if (data[at(node, level - 1, w)] != 0) {
                        data[at(node, level, w >> 6)] |=
                            1ULL << (w & 63);
                    }
                }
            }
        }
    }

    void erase(int row, int col) {
        int node = P + row;
        clearBitInNode(node, col);

        while (node > 1) {
            int sibling = node ^ 1;
            int parent = node >> 1;

            if (hasBit(sibling, col)) break;

            clearBitInNode(parent, col);
            node = parent;
        }
    }

    template <class Callback>
    void extractRectangle(int r1, int r2, int c1, int c2,
                          Callback callback) {
        r1 = max(r1, 0);
        r2 = min(r2, R - 1);
        c1 = max(c1, 0);
        c2 = min(c2, C - 1);

        if (r1 > r2 || c1 > c2) return;

        extractRec(1, 0, P - 1, r1, r2, c1, c2, callback);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int R, C, N;
    cin >> R >> C >> N;

    int sr, sc, gr, gc;
    cin >> sr >> sc;
    cin >> gr >> gc;
    --sr;
    --sc;
    --gr;
    --gc;

    vector<string> grid(R);
    for (string &row : grid) cin >> row;

    const int V = R * C;
    const int start = sr * C + sc;
    const int goal = gr * C + gc;

    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};

    // Specialized linear solution for Subtask 1 and very thin grids.
    if (N == 1) {
        const int INF = numeric_limits<int>::max() / 4;
        vector<int> dist(V, INF);
        deque<int> dq;

        dist[start] = 0;
        dq.push_front(start);

        while (!dq.empty()) {
            int u = dq.front();
            dq.pop_front();

            int r = u / C;
            int c = u % C;
            int du = dist[u];

            for (int dir = 0; dir < 4; ++dir) {
                int nr = r + dr[dir];
                int nc = c + dc[dir];
                if (nr < 0 || nr >= R || nc < 0 || nc >= C) continue;

                int v = nr * C + nc;
                int weight = (grid[nr][nc] == '#') ? 1 : 0;

                if (dist[v] > du + weight) {
                    dist[v] = du + weight;
                    if (weight == 0) dq.push_front(v);
                    else dq.push_back(v);
                }
            }
        }

        cout << dist[goal] << '\n';
        return 0;
    }

    UnvisitedGrid unvisited(R, C);

    vector<int> dist(V, -1);
    vector<unsigned char> processed(V, 0);
    deque<int> dq;

    dist[start] = 0;
    unvisited.erase(sr, sc);
    dq.push_front(start);

    while (!dq.empty()) {
        int u = dq.front();
        dq.pop_front();

        if (processed[u]) continue;
        processed[u] = 1;

        int du = dist[u];
        if (u == goal) {
            cout << du << '\n';
            return 0;
        }

        int r = u / C;
        int c = u % C;

        // Free movement into adjacent originally white cells.
        for (int dir = 0; dir < 4; ++dir) {
            int nr = r + dr[dir];
            int nc = c + dc[dir];
            if (nr < 0 || nr >= R || nc < 0 || nc >= C) continue;
            if (grid[nr][nc] != '.') continue;

            int v = nr * C + nc;
            if (dist[v] == -1) {
                unvisited.erase(nr, nc);
                dist[v] = du;
                dq.push_front(v);
            } else if (dist[v] > du) {
                dist[v] = du;
                dq.push_front(v);
            }
        }

        auto discoverByStamp = [&](int v) {
            dist[v] = du + 1;
            dq.push_back(v);
        };

        // The radius-N square without its four corners.
        unvisited.extractRectangle(
            r - N + 1, r + N - 1,
            c - N, c + N,
            discoverByStamp
        );

        unvisited.extractRectangle(
            r - N, r - N,
            c - N + 1, c + N - 1,
            discoverByStamp
        );

        unvisited.extractRectangle(
            r + N, r + N,
            c - N + 1, c + N - 1,
            discoverByStamp
        );
    }

    return 0;
}
