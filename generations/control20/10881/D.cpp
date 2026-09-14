#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <cstdint>
using namespace std;

class DSU {
    vector<int> parent, size;

public:
    explicit DSU(int n) : parent(n), size(n, 1) {
        iota(parent.begin(), parent.end(), 0);
    }

    int find(int x) {
        return parent[x] == x ? x : parent[x] = find(parent[x]);
    }

    void unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return;

        if (size[a] < size[b]) swap(a, b);
        parent[b] = a;
        size[a] += size[b];
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<vector<uint16_t>> mode(n, vector<uint16_t>(n));
    for (int left = 0; left < n; ++left) {
        for (int right = left; right < n; ++right) {
            int value;
            cin >> value;
            mode[left][right] = static_cast<uint16_t>(value);
        }
    }

    DSU dsu(n);

    for (int left = 0; left < n; ++left) {
        for (int right = left + 1; right < n; ++right) {
            int removeLeft =
                int(mode[left][right]) - int(mode[left + 1][right]);
            int removeRight =
                int(mode[left][right]) - int(mode[left][right - 1]);

            if (removeLeft == 1 && removeRight == 1) {
                dsu.unite(left, right);
            }
        }
    }

    vector<int> rootId(n, -1);
    vector<int> componentOf(n);
    int componentCount = 0;

    for (int position = 0; position < n; ++position) {
        int root = dsu.find(position);
        if (rootId[root] == -1) {
            rootId[root] = componentCount++;
        }
        componentOf[position] = rootId[root];
    }

    vector<int> componentSize(componentCount, 0);
    for (int position = 0; position < n; ++position) {
        ++componentSize[componentOf[position]];
    }

    vector<vector<uint8_t>> different(
        componentCount, vector<uint8_t>(componentCount, 0)
    );

    for (int left = 0; left < n; ++left) {
        for (int right = left + 1; right < n; ++right) {
            int removeLeft =
                int(mode[left][right]) - int(mode[left + 1][right]);
            int removeRight =
                int(mode[left][right]) - int(mode[left][right - 1]);

            if (removeLeft != removeRight) {
                int a = componentOf[left];
                int b = componentOf[right];
                if (a != b) {
                    different[a][b] = different[b][a] = 1;
                }
            }
        }
    }

    int wordCount = (n + 63) / 64;
    vector<vector<uint64_t>> neighbourhood(
        componentCount, vector<uint64_t>(wordCount, 0)
    );

    for (int component = 0; component < componentCount; ++component) {
        for (int position = 0; position < n; ++position) {
            if (different[component][componentOf[position]]) {
                neighbourhood[component][position / 64] |=
                    uint64_t(1) << (position % 64);
            }
        }
    }

    int bestSize = 0;
    int bestComponent = -1;
    int bestFirst = -1;
    int bestSecond = -1;

    for (int component = 0; component < componentCount; ++component) {
        if (componentSize[component] > bestSize) {
            bestSize = componentSize[component];
            bestComponent = component;
            bestFirst = bestSecond = -1;
        }
    }

    for (int first = 0; first < componentCount; ++first) {
        for (int second = first + 1; second < componentCount; ++second) {
            if (!different[first][second]) continue;

            int currentSize = 0;
            for (int word = 0; word < wordCount; ++word) {
                currentSize += __builtin_popcountll(
                    neighbourhood[first][word] &
                    neighbourhood[second][word]
                );
            }

            if (currentSize > bestSize) {
                bestSize = currentSize;
                bestComponent = -1;
                bestFirst = first;
                bestSecond = second;
            }
        }
    }

    bool firstOutput = true;

    for (int position = 0; position < n; ++position) {
        bool isB;

        if (bestComponent != -1) {
            isB = componentOf[position] == bestComponent;
        } else {
            uint64_t bit = uint64_t(1) << (position % 64);
            isB =
                (neighbourhood[bestFirst][position / 64] & bit) &&
                (neighbourhood[bestSecond][position / 64] & bit);
        }

        if (isB) {
            if (!firstOutput) cout << ' ';
            cout << position + 1;
            firstOutput = false;
        }
    }

    cout << '\n';
    return 0;
}
