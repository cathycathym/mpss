#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

struct DSU {
    vector<int> parent, vertices, edges;

    explicit DSU(int n) : parent(n), vertices(n, 1), edges(n, 0) {
        iota(parent.begin(), parent.end(), 0);
    }

    int find(int x) {
        if (parent[x] == x) return x;
        return parent[x] = find(parent[x]);
    }

    void addEdge(int a, int b) {
        a = find(a);
        b = find(b);

        if (a == b) {
            ++edges[a];
            return;
        }

        if (vertices[a] < vertices[b]) swap(a, b);
        parent[b] = a;
        vertices[a] += vertices[b];
        edges[a] += edges[b] + 1;
    }

    int64 validOrientations() {
        int64 result = 1;

        for (int i = 0; i < (int)parent.size(); ++i) {
            if (find(i) != i) continue;

            if (edges[i] == vertices[i] - 1) {
                result *= vertices[i];
            } else if (edges[i] == vertices[i]) {
                result *= 2;
            } else {
                return 0;
            }
        }

        return result;
    }
};

class Solver {
    int n;
    unordered_map<string, int64> memo;

    string encode(const vector<int>& trees, int cyclicCount) const {
        string key;
        key.reserve(trees.size() + 1);
        key.push_back(static_cast<char>(cyclicCount));
        for (int size : trees) {
            key.push_back(static_cast<char>(size));
        }
        return key;
    }

    int64 orientations(const vector<int>& trees, int cyclicCount) const {
        int64 result = 1LL << cyclicCount;
        for (int size : trees) result *= size;
        return result;
    }

    int64 solveState(const vector<int>& trees, int cyclicCount) {
        string key = encode(trees, cyclicCount);
        auto found = memo.find(key);
        if (found != memo.end()) return found->second;

        int treeComponents = (int)trees.size();
        int processedEdges = n - treeComponents;
        int nextMove = processedEdges + 1;

        bool aliceMoves = (nextMove % 2 == 1);
        int64 best = aliceMoves ? LLONG_MIN : LLONG_MAX;
        int64 currentF = orientations(trees, cyclicCount);

        auto consider = [&](const vector<int>& nextTrees,
                            int nextCyclicCount,
                            bool valid) {
            int64 nextF = valid
                ? orientations(nextTrees, nextCyclicCount)
                : 0;

            int64 value = 0;
            if (nextMove % 2 == 0) {
                value =
                    currentF * (1LL << (n + 2 - nextMove))
                    - nextF * (1LL << (n + 1 - nextMove));
            }

            if (valid) {
                value += solveState(nextTrees, nextCyclicCount);
            }

            if (aliceMoves) best = max(best, value);
            else best = min(best, value);
        };

        // Join two tree components: the result is another tree.
        for (int i = 0; i < treeComponents; ++i) {
            if (i > 0 && trees[i] == trees[i - 1]) continue;

            for (int j = i + 1; j < treeComponents; ++j) {
                if (j > i + 1 && trees[j] == trees[j - 1]) continue;

                vector<int> nextTrees;
                nextTrees.reserve(treeComponents - 1);

                for (int x = 0; x < treeComponents; ++x) {
                    if (x != i && x != j) nextTrees.push_back(trees[x]);
                }

                nextTrees.push_back(trees[i] + trees[j]);
                sort(nextTrees.begin(), nextTrees.end());
                consider(nextTrees, cyclicCount, true);
            }
        }

        // Add an edge inside a tree: it becomes unicyclic.
        for (int i = 0; i < treeComponents; ++i) {
            if (i > 0 && trees[i] == trees[i - 1]) continue;

            vector<int> nextTrees;
            nextTrees.reserve(treeComponents - 1);

            for (int x = 0; x < treeComponents; ++x) {
                if (x != i) nextTrees.push_back(trees[x]);
            }

            consider(nextTrees, cyclicCount + 1, true);
        }

        if (cyclicCount > 0) {
            // Join a tree to a unicyclic component.
            for (int i = 0; i < treeComponents; ++i) {
                if (i > 0 && trees[i] == trees[i - 1]) continue;

                vector<int> nextTrees;
                nextTrees.reserve(treeComponents - 1);

                for (int x = 0; x < treeComponents; ++x) {
                    if (x != i) nextTrees.push_back(trees[x]);
                }

                consider(nextTrees, cyclicCount, true);
            }

            // An edge inside a unicyclic component is immediately invalid.
            vector<int> unused;
            consider(unused, 0, false);
        }

        memo.emplace(move(key), best);
        return best;
    }

public:
    explicit Solver(int n_) : n(n_) {
        memo.reserve(600000);
        memo.max_load_factor(0.7f);
    }

    int64 solve(const vector<int>& trees, int cyclicCount) {
        return solveState(trees, cyclicCount);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    cin >> n >> k;

    DSU dsu(n);
    vector<int64> f(k + 1);
    f[0] = 1;

    for (int i = 1; i <= k; ++i) {
        int a, b;
        cin >> a >> b;
        --a;
        --b;
        dsu.addEdge(a, b);
        f[i] = dsu.validOrientations();
    }

    int64 aliceWins = 0;

    // Contributions from failures that already occur in the fixed prefix.
    for (int i = 1; i <= k; ++i) {
        if (i % 2 == 0) {
            aliceWins +=
                f[i - 1] * (1LL << (n + 2 - i))
                - f[i] * (1LL << (n + 1 - i));
        }
    }

    if (f[k] != 0) {
        vector<int> trees;
        int cyclicCount = 0;

        for (int i = 0; i < n; ++i) {
            if (dsu.find(i) != i) continue;

            if (dsu.edges[i] == dsu.vertices[i] - 1) {
                trees.push_back(dsu.vertices[i]);
            } else {
                ++cyclicCount;
            }
        }

        sort(trees.begin(), trees.end());

        Solver solver(n);
        aliceWins += solver.solve(trees, cyclicCount);
    }

    int64 totalScenarios = 1LL << (n + 1);
    int64 bobWins = totalScenarios - aliceWins;

    cout << aliceWins << ' ' << bobWins << '\n';
    return 0;
}