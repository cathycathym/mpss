#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <utility>
#include <cstdint>

using namespace std;

class DSU {
    vector<int> parent, sz;

public:
    explicit DSU(int n) : parent(n), sz(n, 1) {
        for (int i = 0; i < n; ++i) parent[i] = i;
    }

    int find(int x) {
        while (parent[x] != x) {
            parent[x] = parent[parent[x]];
            x = parent[x];
        }
        return x;
    }

    bool unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return false;

        if (sz[a] < sz[b]) swap(a, b);
        parent[b] = a;
        sz[a] += sz[b];
        return true;
    }
};

static void exploreReverseComplement(
    int start,
    list<int>& unvisited,
    const vector<vector<int>>& blockedIncoming,
    vector<int>& mark,
    int& timestamp,
    vector<int>* treeParent
) {
    vector<int> queue;
    queue.push_back(start);

    for (size_t head = 0; head < queue.size(); ++head) {
        int current = queue[head];
        ++timestamp;

        for (int blockedSource : blockedIncoming[current]) {
            mark[blockedSource] = timestamp;
        }

        for (auto it = unvisited.begin(); it != unvisited.end(); ) {
            int next = *it;

            if (mark[next] != timestamp) {
                it = unvisited.erase(it);
                queue.push_back(next);

                if (treeParent != nullptr) {
                    (*treeParent)[next] = current;
                }
            } else {
                ++it;
            }
        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M, K;
    cin >> N >> M >> K;

    vector<pair<int, int>> requiredEdges;
    requiredEdges.reserve(M);

    vector<int> requiredParent(N, -1);
    DSU dsu(N);
    bool valid = true;

    for (int i = 0; i < M; ++i) {
        int a, b;
        cin >> a >> b;
        requiredEdges.push_back({a, b});

        if (requiredParent[a] != -1) {
            valid = false;
        } else {
            requiredParent[a] = b;
        }

        if (!dsu.unite(a, b)) {
            valid = false;
        }
    }

    if (!valid) {
        cout << "NO\n";
        return 0;
    }

    vector<int> leaderToComponent(N, -1);
    vector<int> componentOf(N);
    int componentCount = 0;

    for (int v = 0; v < N; ++v) {
        int leader = dsu.find(v);
        if (leaderToComponent[leader] == -1) {
            leaderToComponent[leader] = componentCount++;
        }
        componentOf[v] = leaderToComponent[leader];
    }

    vector<int> componentTop(componentCount, -1);
    vector<int> componentSize(componentCount, 0);
    vector<vector<int>> componentVertices(componentCount);

    for (int v = 0; v < N; ++v) {
        int c = componentOf[v];
        ++componentSize[c];
        componentVertices[c].push_back(v);

        if (requiredParent[v] == -1) {
            if (componentTop[c] != -1) valid = false;
            componentTop[c] = v;
        }
    }

    for (int c = 0; c < componentCount; ++c) {
        if (componentTop[c] == -1) valid = false;
    }

    if (!valid) {
        cout << "NO\n";
        return 0;
    }

    using U64 = uint64_t;
    vector<U64> originalForbidden;
    vector<U64> contractedForbiddenOccurrences;
    originalForbidden.reserve(K);
    contractedForbiddenOccurrences.reserve(K);

    for (int i = 0; i < K; ++i) {
        int from, to;
        cin >> from >> to;

        originalForbidden.push_back(
            static_cast<U64>(from) * static_cast<U64>(N) +
            static_cast<U64>(to)
        );

        int fromComponent = componentOf[from];
        int toComponent = componentOf[to];

        if (fromComponent != toComponent &&
            componentTop[fromComponent] == from) {
            contractedForbiddenOccurrences.push_back(
                static_cast<U64>(fromComponent) *
                    static_cast<U64>(componentCount) +
                static_cast<U64>(toComponent)
            );
        }
    }

    sort(originalForbidden.begin(), originalForbidden.end());
    sort(contractedForbiddenOccurrences.begin(),
         contractedForbiddenOccurrences.end());

    // blockedIncoming[target] contains every source for which source -> target
    // is completely unavailable.
    vector<vector<int>> blockedIncoming(componentCount);

    for (size_t left = 0;
         left < contractedForbiddenOccurrences.size(); ) {
        size_t right = left + 1;
        while (right < contractedForbiddenOccurrences.size() &&
               contractedForbiddenOccurrences[right] ==
                   contractedForbiddenOccurrences[left]) {
            ++right;
        }

        U64 key = contractedForbiddenOccurrences[left];
        int source = static_cast<int>(key / componentCount);
        int target = static_cast<int>(key % componentCount);
        int prohibitedTargets = static_cast<int>(right - left);

        if (prohibitedTargets == componentSize[target]) {
            blockedIncoming[target].push_back(source);
        }

        left = right;
    }

    vector<int> mark(componentCount, 0);
    int timestamp = 0;

    // Reachability forest in the reverse graph.
    list<int> unvisited;
    for (int c = 0; c < componentCount; ++c) {
        unvisited.push_back(c);
    }

    int candidate = -1;

    while (!unvisited.empty()) {
        int start = unvisited.front();
        unvisited.pop_front();
        candidate = start;

        exploreReverseComplement(
            start, unvisited, blockedIncoming,
            mark, timestamp, nullptr
        );
    }

    // Verify the candidate and simultaneously construct the component tree.
    unvisited.clear();
    for (int c = 0; c < componentCount; ++c) {
        if (c != candidate) unvisited.push_back(c);
    }

    vector<int> componentParent(componentCount, -2);
    componentParent[candidate] = -1;

    exploreReverseComplement(
        candidate, unvisited, blockedIncoming,
        mark, timestamp, &componentParent
    );

    if (!unvisited.empty()) {
        cout << "NO\n";
        return 0;
    }

    vector<pair<int, int>> answer = requiredEdges;
    answer.reserve(N - 1);

    for (int sourceComponent = 0;
         sourceComponent < componentCount;
         ++sourceComponent) {
        if (sourceComponent == candidate) continue;

        int targetComponent = componentParent[sourceComponent];
        int sourceVertex = componentTop[sourceComponent];
        int targetVertex = -1;

        for (int v : componentVertices[targetComponent]) {
            U64 key =
                static_cast<U64>(sourceVertex) * static_cast<U64>(N) +
                static_cast<U64>(v);

            if (!binary_search(originalForbidden.begin(),
                               originalForbidden.end(), key)) {
                targetVertex = v;
                break;
            }
        }

        // The contracted edge was traversed, so an allowed target must exist.
        if (targetVertex == -1) {
            cout << "NO\n";
            return 0;
        }

        answer.push_back({sourceVertex, targetVertex});
    }

    if (static_cast<int>(answer.size()) != N - 1) {
        cout << "NO\n";
        return 0;
    }

    for (const auto& edge : answer) {
        cout << edge.first << ' ' << edge.second << '\n';
    }

    return 0;
}