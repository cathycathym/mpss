#include <algorithm>
#include <climits>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

using int64 = long long;

struct DSU {
    vector<int> parent, vertices, edges;

    explicit DSU(int n) : parent(n), vertices(n, 1), edges(n, 0) {
        iota(parent.begin(), parent.end(), 0);
    }

    int find(int x) {
        return parent[x] == x ? x : parent[x] = find(parent[x]);
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

    int64 validScenarios() {
        int64 result = 1;

        for (int i = 0; i < static_cast<int>(parent.size()); ++i) {
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

class GameSolver {
    int n;
    unordered_map<string, int64> memo;

    string encode(const vector<int>& trees, int cyclicComponents) const {
        string key;
        key.reserve(trees.size() + 1);
        key.push_back(static_cast<char>(cyclicComponents));
        for (int size : trees) {
            key.push_back(static_cast<char>(size));
        }
        return key;
    }

    int64 survivingScenarios(const vector<int>& trees,
                             int cyclicComponents) const {
        int64 result = 1LL << cyclicComponents;
        for (int size : trees) result *= size;
        return result;
    }

    int64 solveState(const vector<int>& trees, int cyclicComponents) {
        string key = encode(trees, cyclicComponents);
        auto it = memo.find(key);
        if (it != memo.end()) return it->second;

        const int treeCount = static_cast<int>(trees.size());
        const int processedMoves = n - treeCount;
        const int move = processedMoves + 1;
        const bool aliceMoves = (move % 2 == 1);

        const int64 currentSurvivors =
            survivingScenarios(trees, cyclicComponents);

        int64 best = aliceMoves ? LLONG_MIN : LLONG_MAX;

        auto consider = [&](const vector<int>& nextTrees,
                            int nextCyclicComponents,
                            bool remainsValid) {
            int64 nextSurvivors = 0;
            if (remainsValid) {
                nextSurvivors =
                    survivingScenarios(nextTrees, nextCyclicComponents);
            }

            int64 aliceWins = 0;

            if (move % 2 == 0) {
                const int remainingMoves = n + 1 - move;
                aliceWins +=
                    (2 * currentSurvivors - nextSurvivors)
                    * (1LL << remainingMoves);
            }

            if (remainsValid) {
                aliceWins += solveState(nextTrees, nextCyclicComponents);
            }

            if (aliceMoves) {
                best = max(best, aliceWins);
            } else {
                best = min(best, aliceWins);
            }
        };

        // Join two tree components.
        for (int i = 0; i < treeCount; ++i) {
            if (i > 0 && trees[i] == trees[i - 1]) continue;

            for (int j = i + 1; j < treeCount; ++j) {
                if (j > i + 1 && trees[j] == trees[j - 1]) continue;

                vector<int> nextTrees;
                nextTrees.reserve(treeCount - 1);

                for (int p = 0; p < treeCount; ++p) {
                    if (p != i && p != j) nextTrees.push_back(trees[p]);
                }

                nextTrees.push_back(trees[i] + trees[j]);
                sort(nextTrees.begin(), nextTrees.end());

                consider(nextTrees, cyclicComponents, true);
            }
        }

        // Add an edge within a tree, making it unicyclic.
        for (int i = 0; i < treeCount; ++i) {
            if (i > 0 && trees[i] == trees[i - 1]) continue;

            vector<int> nextTrees;
            nextTrees.reserve(treeCount - 1);

            for (int p = 0; p < treeCount; ++p) {
                if (p != i) nextTrees.push_back(trees[p]);
            }

            consider(nextTrees, cyclicComponents + 1, true);
        }

        if (cyclicComponents > 0) {
            // Attach a tree to a unicyclic component.
            for (int i = 0; i < treeCount; ++i) {
                if (i > 0 && trees[i] == trees[i - 1]) continue;

                vector<int> nextTrees;
                nextTrees.reserve(treeCount - 1);

                for (int p = 0; p < treeCount; ++p) {
                    if (p != i) nextTrees.push_back(trees[p]);
                }

                consider(nextTrees, cyclicComponents, true);
            }

            // Any edge that creates a component with more edges than vertices
            // makes every still-surviving scenario fail on this move.
            const vector<int> empty;
            consider(empty, 0, false);
        }

        memo.emplace(std::move(key), best);
        return best;
    }

public:
    explicit GameSolver(int n) : n(n) {
        memo.reserve(600000);
        memo.max_load_factor(0.7f);
    }

    int64 solve(const vector<int>& trees, int cyclicComponents) {
        return solveState(trees, cyclicComponents);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    cin >> n >> k;

    DSU dsu(n);
    vector<int64> survivors(k + 1);
    survivors[0] = 1;

    for (int move = 1; move <= k; ++move) {
        int a, b;
        cin >> a >> b;
        dsu.addEdge(a - 1, b - 1);
        survivors[move] = dsu.validScenarios();
    }

    int64 aliceWins = 0;

    for (int move = 1; move <= k; ++move) {
        if (move % 2 == 0) {
            const int remainingMoves = n + 1 - move;
            aliceWins +=
                (2 * survivors[move - 1] - survivors[move])
                * (1LL << remainingMoves);
        }
    }

    if (survivors[k] != 0 && k < n + 1) {
        vector<int> treeSizes;
        int cyclicComponents = 0;

        for (int i = 0; i < n; ++i) {
            if (dsu.find(i) != i) continue;

            if (dsu.edges[i] == dsu.vertices[i] - 1) {
                treeSizes.push_back(dsu.vertices[i]);
            } else {
                ++cyclicComponents;
            }
        }

        sort(treeSizes.begin(), treeSizes.end());

        GameSolver solver(n);
        aliceWins += solver.solve(treeSizes, cyclicComponents);
    }

    const int64 totalScenarios = 1LL << (n + 1);
    cout << aliceWins << ' ' << totalScenarios - aliceWins << '\n';
    return 0;
}