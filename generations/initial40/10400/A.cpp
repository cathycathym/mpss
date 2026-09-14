#include <algorithm>
#include <climits>
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

    int64 survivingCount() {
        int n = static_cast<int>(parent.size());
        int64 result = 1;

        for (int i = 0; i < n; ++i) {
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

int n;
const int N_OFFSET = 1;
unordered_map<string, int64> memo;

string encodeState(const vector<int>& trees, int cycles) {
    string key;
    key.reserve(trees.size() + N_OFFSET);
    key.push_back(static_cast<char>(cycles));
    for (int x : trees) key.push_back(static_cast<char>(x));
    return key;
}

int64 stateSurvivors(const vector<int>& trees, int cycles) {
    int64 result = 1LL << cycles;
    for (int x : trees) result *= x;
    return result;
}

// Returns the number of future complete scenarios won by Alice.
int64 solve(const vector<int>& trees, int cycles) {
    string key = encodeState(trees, cycles);
    auto found = memo.find(key);
    if (found != memo.end()) return found->second;

    int treeCount = static_cast<int>(trees.size());

    // A pseudoforest with treeCount tree components has n-treeCount edges.
    int move = n - treeCount + 1;
    int totalMoves = n + 1;
    int64 survivors = stateSurvivors(trees, cycles);

    bool aliceMoves = (move % 2 == 1);
    int64 best = aliceMoves ? LLONG_MIN : LLONG_MAX;

    auto consider = [&](int64 nextSurvivors,
                        const vector<int>* nextTrees,
                        int nextCycles,
                        bool becomesInvalid) {
        int64 value = 0;

        if (move % 2 == 0) {
            int64 failures = 2 * survivors - nextSurvivors;
            value += failures * (1LL << (totalMoves - move));
        }

        if (!becomesInvalid) {
            value += solve(*nextTrees, nextCycles);
        }

        if (aliceMoves) best = max(best, value);
        else best = min(best, value);
    };

    // Add an edge inside a unicyclic component. This makes survival impossible.
    if (cycles > 0) {
        consider(0, nullptr, 0, true);
    }

    // Process one representative of every distinct tree size.
    for (int i = 0; i < treeCount; ++i) {
        if (i > 0 && trees[i] == trees[i - 1]) continue;

        int a = trees[i];
        vector<int> nextTrees = trees;
        nextTrees.erase(nextTrees.begin() + i);

        // Add an edge inside this tree: it becomes unicyclic.
        consider(survivors / a * 2, &nextTrees, cycles + 1, false);

        // Attach this tree to any unicyclic component.
        if (cycles > 0) {
            consider(survivors / a, &nextTrees, cycles, false);
        }
    }

    // Merge two tree components.
    for (int i = 0; i < treeCount; ++i) {
        if (i > 0 && trees[i] == trees[i - 1]) continue;

        for (int j = i + 1; j < treeCount; ++j) {
            if (j > i + 1 && trees[j] == trees[j - 1]) continue;

            int a = trees[i];
            int b = trees[j];

            vector<int> nextTrees;
            nextTrees.reserve(treeCount - 1);

            for (int p = 0; p < treeCount; ++p) {
                if (p != i && p != j) nextTrees.push_back(trees[p]);
            }

            nextTrees.push_back(a + b);
            sort(nextTrees.begin(), nextTrees.end());

            int64 nextSurvivors = survivors / a / b * (a + b);
            consider(nextSurvivors, &nextTrees, cycles, false);
        }
    }

    memo.emplace(std::move(key), best);
    return best;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int k;
    cin >> n >> k;

    DSU dsu(n);
    int totalMoves = n + 1;

    int64 aliceWins = 0;
    int64 previousSurvivors = 1;

    for (int move = 1; move <= k; ++move) {
        int a, b;
        cin >> a >> b;
        --a;
        --b;

        dsu.addEdge(a, b);
        int64 currentSurvivors = dsu.survivingCount();

        int64 failures =
            previousSurvivors * (1LL << (totalMoves - move + 1))
            - currentSurvivors * (1LL << (totalMoves - move));

        if (move % 2 == 0) aliceWins += failures;
        previousSurvivors = currentSurvivors;
    }

    int64 totalScenarios = 1LL << totalMoves;

    // Once no valid orientation remains, every scenario has already ended.
    if (previousSurvivors == 0 || k == totalMoves) {
        cout << aliceWins << ' ' << totalScenarios - aliceWins << '\n';
        return 0;
    }

    vector<int> trees;
    int cycles = 0;

    for (int i = 0; i < n; ++i) {
        if (dsu.find(i) != i) continue;

        if (dsu.edges[i] == dsu.vertices[i] - 1) {
            trees.push_back(dsu.vertices[i]);
        } else {
            ++cycles;
        }
    }

    sort(trees.begin(), trees.end());

    memo.reserve(500000);
    aliceWins += solve(trees, cycles);

    cout << aliceWins << ' ' << totalScenarios - aliceWins << '\n';
    return 0;
}