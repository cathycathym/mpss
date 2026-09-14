#include <iostream>
#include <vector>
#include <cstdint>
#include <numeric>
#include <algorithm>

using namespace std;

class DSU {
    vector<int> parent, sz;

public:
    explicit DSU(int n) : parent(n), sz(n, 1) {
        iota(parent.begin(), parent.end(), 0);
    }

    int find(int x) {
        if (parent[x] == x) return x;
        return parent[x] = find(parent[x]);
    }

    void unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return;

        if (sz[a] < sz[b]) swap(a, b);
        parent[b] = a;
        sz[a] += sz[b];
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<vector<uint16_t>> M(n, vector<uint16_t>(n));
    for (int l = 0; l < n; ++l) {
        for (int r = l; r < n; ++r) {
            int x;
            cin >> x;
            M[l][r] = static_cast<uint16_t>(x);
        }
    }

    DSU dsu(n);

    // Intervals whose two endpoints are both certified as the unique mode
    // prove that the endpoints contain the same character.
    for (int l = 0; l < n; ++l) {
        for (int r = l + 1; r < n; ++r) {
            int leftDrop = int(M[l][r]) - int(M[l + 1][r]);
            int rightDrop = int(M[l][r]) - int(M[l][r - 1]);

            if (leftDrop == 1 && rightDrop == 1) {
                dsu.unite(l, r);
            }
        }
    }

    vector<int> rootToComponent(n, -1);
    vector<int> componentOf(n);
    int components = 0;

    for (int i = 0; i < n; ++i) {
        int root = dsu.find(i);
        if (rootToComponent[root] == -1) {
            rootToComponent[root] = components++;
        }
        componentOf[i] = rootToComponent[root];
    }

    vector<int> componentSize(components, 0);
    for (int i = 0; i < n; ++i) {
        ++componentSize[componentOf[i]];
    }

    // inequality[u][v] means that the two monochromatic components
    // are certified to contain different characters.
    vector<vector<uint8_t>> inequality(
        components, vector<uint8_t>(components, 0)
    );

    for (int l = 0; l < n; ++l) {
        for (int r = l + 1; r < n; ++r) {
            int leftDrop = int(M[l][r]) - int(M[l + 1][r]);
            int rightDrop = int(M[l][r]) - int(M[l][r - 1]);

            if (leftDrop != rightDrop) {
                int a = componentOf[l];
                int b = componentOf[r];
                if (a != b) {
                    inequality[a][b] = 1;
                    inequality[b][a] = 1;
                }
            }
        }
    }

    int words = (n + 63) / 64;

    // neighbourhood[u] is expanded to original positions:
    // bit p is set iff component u is known to differ from position p.
    vector<vector<uint64_t>> neighbourhood(
        components, vector<uint64_t>(words, 0)
    );

    for (int u = 0; u < components; ++u) {
        for (int p = 0; p < n; ++p) {
            if (inequality[u][componentOf[p]]) {
                neighbourhood[u][p >> 6] |= 1ULL << (p & 63);
            }
        }
    }

    // A candidate is either one equality component or the common
    // neighbourhood of the endpoints of an inequality edge.
    int bestSize = 0;
    int bestComponent = -1;
    int bestU = -1, bestV = -1;

    for (int c = 0; c < components; ++c) {
        if (componentSize[c] > bestSize) {
            bestSize = componentSize[c];
            bestComponent = c;
            bestU = bestV = -1;
        }
    }

    for (int u = 0; u < components; ++u) {
        for (int v = u + 1; v < components; ++v) {
            if (!inequality[u][v]) continue;

            int currentSize = 0;
            for (int w = 0; w < words; ++w) {
                currentSize += __builtin_popcountll(
                    neighbourhood[u][w] & neighbourhood[v][w]
                );
            }

            if (currentSize > bestSize) {
                bestSize = currentSize;
                bestComponent = -1;
                bestU = u;
                bestV = v;
            }
        }
    }

    bool first = true;

    for (int p = 0; p < n; ++p) {
        bool chosen;

        if (bestComponent != -1) {
            chosen = (componentOf[p] == bestComponent);
        } else {
            uint64_t mask = 1ULL << (p & 63);
            chosen =
                (neighbourhood[bestU][p >> 6] & mask) &&
                (neighbourhood[bestV][p >> 6] & mask);
        }

        if (chosen) {
            if (!first) cout << ' ';
            cout << p + 1;
            first = false;
        }
    }

    cout << '\n';
    return 0;
}
