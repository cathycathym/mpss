#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
using u64 = unsigned long long;

struct Edge {
    int u, v;
    int64 w;

    bool operator<(const Edge& other) const {
        return w < other.w;
    }
};

struct Query {
    int l, r, id;
    int64 order;
};

static int64 hilbertOrder(int x, int y, int power, int rotation = 0) {
    if (power == 0) return 0;

    int half = 1 << (power - 1);
    int segment;

    if (x < half) {
        segment = (y < half ? 0 : 3);
    } else {
        segment = (y < half ? 1 : 2);
    }

    segment = (segment + rotation) & 3;

    static const int rotationDelta[4] = {3, 0, 0, 1};

    int nx = x & (half - 1);
    int ny = y & (half - 1);
    int newRotation = (rotation + rotationDelta[segment]) & 3;

    int64 subSquareSize = 1LL << (2 * power - 2);
    int64 result = segment * subSquareSize;
    int64 subOrder = hilbertOrder(nx, ny, power - 1, newRotation);

    if (segment == 1 || segment == 2) {
        return result + subOrder;
    }
    return result + subSquareSize - subOrder - 1;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m, q;
    cin >> n >> m >> q;

    vector<Edge> edges(m);
    for (Edge& e : edges) {
        cin >> e.u >> e.v >> e.w;
    }
    sort(edges.begin(), edges.end());

    /*
      Build a binary Kruskal reconstruction tree.

      Leaves 0..n-1 are the original vertices. Every successful Kruskal merge
      creates an internal node whose value is the weight of the merging edge.
      For two leaves, the value of their LCA is their minimum bottleneck
      distance.
    */
    vector<int> dsu(n), componentNode(n);
    iota(dsu.begin(), dsu.end(), 0);
    iota(componentNode.begin(), componentNode.end(), 0);

    auto findRoot = [&](int x) {
        int root = x;
        while (dsu[root] != root) root = dsu[root];

        while (dsu[x] != x) {
            int parent = dsu[x];
            dsu[x] = root;
            x = parent;
        }
        return root;
    };

    vector<int> leftChild(2 * n, -1);
    vector<int> rightChild(2 * n, -1);
    vector<int64> nodeValue(2 * n, 0);

    int nodeCount = n;
    int64 mstWeight = 0;

    for (const Edge& e : edges) {
        int a = findRoot(e.u);
        int b = findRoot(e.v);
        if (a == b) continue;

        int node = nodeCount++;
        leftChild[node] = componentNode[a];
        rightChild[node] = componentNode[b];
        nodeValue[node] = e.w;

        dsu[b] = a;
        componentNode[a] = node;
        mstWeight += e.w;
    }

    int root = componentNode[findRoot(0)];

    /*
      Euler tour of the reconstruction tree for constant-time LCA queries.
      The DFS order of leaves is also used to find the closest active terminal:
      in a tree ultrametric, one of its active predecessor/successor leaves is
      always closest.
    */
    vector<int> depth(nodeCount, 0);
    vector<int> first(nodeCount, -1);
    vector<int> leafPosition(n);
    vector<int> leafAtPosition(n);
    vector<int> euler;
    euler.reserve(2 * nodeCount);

    vector<pair<int, int>> stack;
    stack.push_back({root, 0});

    int leafCount = 0;

    while (!stack.empty()) {
        int node = stack.back().first;
        int state = stack.back().second;

        if (state == 0) {
            first[node] = static_cast<int>(euler.size());
            euler.push_back(node);

            if (node < n) {
                leafPosition[node] = leafCount;
                leafAtPosition[leafCount] = node;
                ++leafCount;
                stack.pop_back();
            } else {
                stack.back().second = 1;
                int child = leftChild[node];
                depth[child] = depth[node] + 1;
                stack.push_back({child, 0});
            }
        } else if (state == 1) {
            euler.push_back(node);
            stack.back().second = 2;

            int child = rightChild[node];
            depth[child] = depth[node] + 1;
            stack.push_back({child, 0});
        } else {
            euler.push_back(node);
            stack.pop_back();
        }
    }

    int eulerSize = static_cast<int>(euler.size());
    vector<int> logarithm(eulerSize + 1, 0);

    for (int i = 2; i <= eulerSize; ++i) {
        logarithm[i] = logarithm[i / 2] + 1;
    }

    int logCount = logarithm[eulerSize] + 1;
    vector<vector<int>> sparse(logCount, vector<int>(eulerSize));
    sparse[0] = euler;

    for (int level = 1; level < logCount; ++level) {
        int length = 1 << level;
        int half = length >> 1;

        for (int i = 0; i + length <= eulerSize; ++i) {
            int a = sparse[level - 1][i];
            int b = sparse[level - 1][i + half];
            sparse[level][i] = (depth[a] < depth[b] ? a : b);
        }
    }

    auto lca = [&](int a, int b) {
        int left = first[a];
        int right = first[b];
        if (left > right) swap(left, right);

        int level = logarithm[right - left + 1];
        int x = sparse[level][left];
        int y = sparse[level][right - (1 << level) + 1];

        return depth[x] < depth[y] ? x : y;
    };

    /*
      Hierarchical bitset storing the currently active leaves in DFS order.
      Higher levels mark which words of the preceding level are nonempty.
    */
    vector<vector<u64>> bits;
    int words = (n + 63) / 64;

    while (true) {
        bits.push_back(vector<u64>(words, 0));
        if (words == 1) break;
        words = (words + 63) / 64;
    }

    auto previousMarked = [&](auto&& self, int level, int limit) -> int {
        if (limit <= 0 || level >= static_cast<int>(bits.size())) {
            return -1;
        }

        int word = limit >> 6;
        int bit = limit & 63;

        if (word < static_cast<int>(bits[level].size()) && bit != 0) {
            u64 mask = bits[level][word] & ((1ULL << bit) - 1);
            if (mask != 0) {
                return (word << 6) + 63 - __builtin_clzll(mask);
            }
        }

        int previousWord = self(self, level + 1, word);
        if (previousWord < 0) return -1;

        u64 value = bits[level][previousWord];
        return (previousWord << 6) + 63 - __builtin_clzll(value);
    };

    auto nextMarked = [&](auto&& self, int level, int position) -> int {
        if (level >= static_cast<int>(bits.size())) {
            return -1;
        }

        int word = position >> 6;
        int bit = position & 63;

        if (word < static_cast<int>(bits[level].size()) && bit != 63) {
            u64 mask = bits[level][word] & (~0ULL << (bit + 1));
            if (mask != 0) {
                return (word << 6) + __builtin_ctzll(mask);
            }
        }

        int nextWord = self(self, level + 1, word);
        if (nextWord < 0) return -1;

        u64 value = bits[level][nextWord];
        return (nextWord << 6) + __builtin_ctzll(value);
    };

    auto setMarked = [&](int position, bool enabled) {
        int index = position;
        bool desiredState = enabled;

        for (int level = 0; level < static_cast<int>(bits.size()); ++level) {
            int word = index >> 6;
            int bit = index & 63;

            bool wasNonempty = bits[level][word] != 0;

            if (desiredState) {
                bits[level][word] |= 1ULL << bit;
            } else {
                bits[level][word] &= ~(1ULL << bit);
            }

            bool isNonempty = bits[level][word] != 0;
            if (wasNonempty == isNonempty) break;

            index = word;
            desiredState = isNonempty;
        }
    };

    vector<Query> queries(q);

    int hilbertPower = 0;
    while ((1 << hilbertPower) < n) ++hilbertPower;

    for (int i = 0; i < q; ++i) {
        cin >> queries[i].l >> queries[i].r;
        queries[i].id = i;
        queries[i].order =
            hilbertOrder(queries[i].l, queries[i].r, hilbertPower);
    }

    sort(queries.begin(), queries.end(),
         [](const Query& a, const Query& b) {
             return a.order < b.order;
         });

    int activeCount = 0;
    int64 savedWeight = 0;
    vector<int64> answers(q);

    auto closestBottleneck = [&](int vertex) -> int64 {
        int position = leafPosition[vertex];
        int bestNode = -1;

        int predecessor = previousMarked(previousMarked, 0, position);
        if (predecessor >= 0) {
            bestNode = lca(vertex, leafAtPosition[predecessor]);
        }

        int successor = nextMarked(nextMarked, 0, position);
        if (successor >= 0) {
            int candidate = lca(vertex, leafAtPosition[successor]);
            if (bestNode < 0 ||
                nodeValue[candidate] < nodeValue[bestNode]) {
                bestNode = candidate;
            }
        }

        return nodeValue[bestNode];
    };

    auto addVertex = [&](int vertex) {
        if (activeCount > 0) {
            savedWeight += closestBottleneck(vertex);
        }

        setMarked(leafPosition[vertex], true);
        ++activeCount;
    };

    auto removeVertex = [&](int vertex) {
        setMarked(leafPosition[vertex], false);
        --activeCount;

        if (activeCount > 0) {
            savedWeight -= closestBottleneck(vertex);
        }
    };

    int currentLeft = 0;
    int currentRight = -1;

    for (const Query& query : queries) {
        while (currentLeft > query.l) {
            addVertex(--currentLeft);
        }
        while (currentRight < query.r) {
            addVertex(++currentRight);
        }
        while (currentLeft < query.l) {
            removeVertex(currentLeft++);
        }
        while (currentRight > query.r) {
            removeVertex(currentRight--);
        }

        answers[query.id] = mstWeight - savedWeight;
    }

    for (int64 answer : answers) {
        cout << answer << '\n';
    }

    return 0;
}