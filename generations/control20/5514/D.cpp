#include <bits/stdc++.h>
using namespace std;

class ActiveCells {
    int rows, cols, base, nodes;
    vector<int> width;
    vector<vector<uint64_t>> bits;

    bool hasBit(int node, int col) const {
        return (bits[0][static_cast<size_t>(node) * width[0] + col / 64]
                >> (col % 64)) & 1ULL;
    }

    void clearBit(int node, int col) {
        int pos = col;

        for (int level = 0; level < static_cast<int>(bits.size()); ++level) {
            int word = pos / 64;
            int offset = pos % 64;
            uint64_t &value =
                bits[level][static_cast<size_t>(node) * width[level] + word];

            uint64_t mask = 1ULL << offset;
            if ((value & mask) == 0) return;

            value &= ~mask;
            if (value != 0) return;

            pos = word;
        }
    }

    int findNext(int node, int level, int left, int right) const {
        if (left > right) return -1;

        int firstWord = left / 64;
        int lastWord = right / 64;
        int firstOffset = left % 64;
        int lastOffset = right % 64;

        const uint64_t *data =
            bits[level].data() + static_cast<size_t>(node) * width[level];

        if (firstWord == lastWord) {
            uint64_t lowMask = ~0ULL << firstOffset;
            uint64_t highMask =
                lastOffset == 63
                    ? ~0ULL
                    : ((1ULL << (lastOffset + 1)) - 1);

            uint64_t value = data[firstWord] & lowMask & highMask;
            if (value == 0) return -1;
            return firstWord * 64 + __builtin_ctzll(value);
        }

        uint64_t firstValue =
            data[firstWord] & (~0ULL << firstOffset);
        if (firstValue != 0) {
            return firstWord * 64 + __builtin_ctzll(firstValue);
        }

        if (firstWord + 1 <= lastWord - 1) {
            int middleWord =
                findNext(node, level + 1, firstWord + 1, lastWord - 1);

            if (middleWord != -1) {
                uint64_t value = data[middleWord];
                return middleWord * 64 + __builtin_ctzll(value);
            }
        }

        uint64_t highMask =
            lastOffset == 63
                ? ~0ULL
                : ((1ULL << (lastOffset + 1)) - 1);

        uint64_t lastValue = data[lastWord] & highMask;
        if (lastValue != 0) {
            return lastWord * 64 + __builtin_ctzll(lastValue);
        }

        return -1;
    }

public:
    ActiveCells(int r, int c) : rows(r), cols(c) {
        base = 1;
        while (base < rows) base <<= 1;
        nodes = 2 * base;

        int w = (cols + 63) / 64;
        width.push_back(w);
        while (w > 1) {
            w = (w + 63) / 64;
            width.push_back(w);
        }

        bits.resize(width.size());
        for (int level = 0; level < static_cast<int>(width.size()); ++level) {
            bits[level].assign(
                static_cast<size_t>(nodes) * width[level], 0
            );
        }

        uint64_t tailMask =
            cols % 64 == 0
                ? ~0ULL
                : ((1ULL << (cols % 64)) - 1);

        for (int row = 0; row < rows; ++row) {
            int node = base + row;
            uint64_t *data =
                bits[0].data() + static_cast<size_t>(node) * width[0];

            fill(data, data + width[0], ~0ULL);
            data[width[0] - 1] &= tailMask;
        }

        for (int node = base - 1; node >= 1; --node) {
            for (int word = 0; word < width[0]; ++word) {
                bits[0][static_cast<size_t>(node) * width[0] + word] =
                    bits[0][static_cast<size_t>(node * 2) * width[0] + word] |
                    bits[0][static_cast<size_t>(node * 2 + 1) * width[0] + word];
            }
        }

        for (int level = 1; level < static_cast<int>(width.size()); ++level) {
            int previousWidth = width[level - 1];
            int currentWidth = width[level];

            for (int node = 1; node < nodes; ++node) {
                const uint64_t *previous =
                    bits[level - 1].data() +
                    static_cast<size_t>(node) * previousWidth;

                uint64_t *current =
                    bits[level].data() +
                    static_cast<size_t>(node) * currentWidth;

                for (int i = 0; i < previousWidth; ++i) {
                    if (previous[i] != 0) {
                        current[i / 64] |= 1ULL << (i % 64);
                    }
                }
            }
        }
    }

    void erase(int row, int col) {
        int node = base + row;
        if (!hasBit(node, col)) return;

        clearBit(node, col);
        node >>= 1;

        while (node >= 1) {
            if (hasBit(node * 2, col) ||
                hasBit(node * 2 + 1, col)) {
                break;
            }

            clearBit(node, col);
            node >>= 1;
        }
    }

    int findOne(int rowLeft, int rowRight,
                int colLeft, int colRight) const {
        int left = rowLeft + base;
        int right = rowRight + base;

        while (left <= right) {
            if (left & 1) {
                int col = findNext(left, 0, colLeft, colRight);
                if (col != -1) {
                    int node = left;
                    while (node < base) {
                        if (hasBit(node * 2, col)) {
                            node *= 2;
                        } else {
                            node = node * 2 + 1;
                        }
                    }
                    return (node - base) * cols + col;
                }
                ++left;
            }

            if (!(right & 1)) {
                int col = findNext(right, 0, colLeft, colRight);
                if (col != -1) {
                    int node = right;
                    while (node < base) {
                        if (hasBit(node * 2, col)) {
                            node *= 2;
                        } else {
                            node = node * 2 + 1;
                        }
                    }
                    return (node - base) * cols + col;
                }
                --right;
            }

            left >>= 1;
            right >>= 1;
        }

        return -1;
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
    for (int i = 0; i < R; ++i) {
        cin >> grid[i];
    }

    const int total = R * C;
    const int start = Sr * C + Sc;
    const int goal = Gr * C + Gc;

    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};

    if (N == 1) {
        const int INF = numeric_limits<int>::max() / 4;

        vector<int> dist(total, INF);
        vector<unsigned char> processed(total, 0);
        deque<int> queue;

        dist[start] = 0;
        queue.push_front(start);

        while (!queue.empty()) {
            int id = queue.front();
            queue.pop_front();

            if (processed[id]) continue;
            processed[id] = 1;

            if (id == goal) {
                cout << dist[id] << '\n';
                return 0;
            }

            int row = id / C;
            int col = id % C;

            for (int direction = 0; direction < 4; ++direction) {
                int nr = row + dr[direction];
                int nc = col + dc[direction];

                if (nr < 0 || nr >= R || nc < 0 || nc >= C) {
                    continue;
                }

                int next = nr * C + nc;
                int weight = (grid[nr][nc] == '#') ? 1 : 0;

                if (dist[next] > dist[id] + weight) {
                    dist[next] = dist[id] + weight;
                    if (weight == 0) {
                        queue.push_front(next);
                    } else {
                        queue.push_back(next);
                    }
                }
            }
        }

        return 0;
    }

    vector<int> dist(total, -1);
    vector<unsigned char> processed(total, 0);
    deque<int> queue;
    ActiveCells active(R, C);

    dist[start] = 0;
    active.erase(Sr, Sc);
    queue.push_front(start);

    while (!queue.empty()) {
        int id = queue.front();
        queue.pop_front();

        if (processed[id]) continue;
        processed[id] = 1;

        int currentDistance = dist[id];

        if (id == goal) {
            cout << currentDistance << '\n';
            return 0;
        }

        int row = id / C;
        int col = id % C;

        for (int direction = 0; direction < 4; ++direction) {
            int nr = row + dr[direction];
            int nc = col + dc[direction];

            if (nr < 0 || nr >= R || nc < 0 || nc >= C) {
                continue;
            }
            if (grid[nr][nc] != '.') continue;

            int next = nr * C + nc;

            if (dist[next] == -1) {
                active.erase(nr, nc);
                dist[next] = currentDistance;
                queue.push_front(next);
            } else if (dist[next] > currentDistance) {
                dist[next] = currentDistance;
                queue.push_front(next);
            }
        }

        int nextDistance = currentDistance + 1;

        auto discoverRectangle =
            [&](int rowLeft, int rowRight,
                int colLeft, int colRight) {
                rowLeft = max(rowLeft, 0);
                rowRight = min(rowRight, R - 1);
                colLeft = max(colLeft, 0);
                colRight = min(colRight, C - 1);

                if (rowLeft > rowRight || colLeft > colRight) {
                    return;
                }

                while (true) {
                    int next = active.findOne(
                        rowLeft, rowRight, colLeft, colRight
                    );
                    if (next == -1) break;

                    int nr = next / C;
                    int nc = next % C;

                    active.erase(nr, nc);
                    dist[next] = nextDistance;
                    queue.push_back(next);
                }
            };

        discoverRectangle(
            row - N, row + N,
            col - (N - 1), col + (N - 1)
        );

        discoverRectangle(
            row - (N - 1), row + (N - 1),
            col - N, col + N
        );
    }

    return 0;
}
