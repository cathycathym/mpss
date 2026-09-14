#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <utility>

using namespace std;

class DSU {
    vector<int> parent, size_;

public:
    explicit DSU(int n) : parent(n), size_(n, 1) {
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

        if (size_[a] < size_[b]) swap(a, b);
        parent[b] = a;
        size_[a] += size_[b];
        return true;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M, K;
    cin >> N >> M >> K;

    vector<pair<int, int>> mandatory(M);
    vector<int> outgoing(N, -1);
    DSU dsu(N);
    bool invalid = false;

    for (int i = 0; i < M; ++i) {
        int a, b;
        cin >> a >> b;
        mandatory[i] = {a, b};

        if (outgoing[a] != -1) invalid = true;
        else outgoing[a] = b;

        if (!dsu.unite(a, b)) invalid = true;
    }

    vector<pair<int, int>> forbidden(K);
    for (int i = 0; i < K; ++i) {
        cin >> forbidden[i].first >> forbidden[i].second;
    }

    if (invalid) {
        cout << "NO\n";
        return 0;
    }

    // Compress the connected components of mandatory edges.
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

    vector<vector<int>> members(componentCount);
    vector<int> sink(componentCount, -1);

    for (int v = 0; v < N; ++v) {
        int c = componentOf[v];
        members[c].push_back(v);
        if (outgoing[v] == -1) {
            if (sink[c] != -1) invalid = true;
            sink[c] = v;
        }
    }

    for (int c = 0; c < componentCount; ++c) {
        if (sink[c] == -1) invalid = true;
    }

    if (invalid) {
        cout << "NO\n";
        return 0;
    }

    if (componentCount == 1) {
        for (const auto &edge : mandatory) {
            cout << edge.first << ' ' << edge.second << '\n';
        }
        return 0;
    }

    /*
     * incomingForbidden[targetComponent] contains a source component once
     * for every forbidden pair from that component's sink to a building in
     * targetComponent.
     */
    vector<vector<int>> incomingForbidden(componentCount);
    vector<vector<int>> forbiddenTargets(componentCount);

    for (const auto &edge : forbidden) {
        int from = edge.first;
        int to = edge.second;
        int sourceComponent = componentOf[from];
        int targetComponent = componentOf[to];

        // Only component sinks can be used as new edge sources.
        if (from != sink[sourceComponent]) continue;

        // New edges never need to stay inside a mandatory component.
        if (sourceComponent == targetComponent) continue;

        incomingForbidden[targetComponent].push_back(sourceComponent);
        forbiddenTargets[sourceComponent].push_back(to);
    }

    for (int c = 0; c < componentCount; ++c) {
        sort(forbiddenTargets[c].begin(), forbiddenTargets[c].end());
    }

    vector<int> forbiddenCount(componentCount, 0);
    vector<int> seen(componentCount, 0);
    int currentStamp = 0;

    /*
     * Finds all active components that can reach rootComponent.
     * If constructEdges is true, also records one connecting edge for each
     * newly reached component.
     */
    auto runSearch = [&](int rootComponent,
                         const vector<char> &active,
                         bool constructEdges,
                         vector<pair<int, int>> &addedEdges) {
        ++currentStamp;

        priority_queue<pair<int, int>,
                       vector<pair<int, int>>,
                       greater<pair<int, int>>> heap;

        for (int c = 0; c < componentCount; ++c) {
            if (active[c]) {
                forbiddenCount[c] = 0;
                heap.push({0, c});
            }
        }

        vector<int> reached;
        vector<int> targetVertices;
        int targetVertexCount = 0;

        auto addComponent = [&](int component) {
            seen[component] = currentStamp;
            reached.push_back(component);
            targetVertexCount += static_cast<int>(members[component].size());

            if (constructEdges) {
                for (int v : members[component]) {
                    targetVertices.push_back(v);
                }
            }

            for (int source : incomingForbidden[component]) {
                if (active[source] && seen[source] != currentStamp) {
                    ++forbiddenCount[source];
                    heap.push({forbiddenCount[source], source});
                }
            }
        };

        addComponent(rootComponent);

        while (true) {
            while (!heap.empty()) {
                int value = heap.top().first;
                int component = heap.top().second;

                if (!active[component] ||
                    seen[component] == currentStamp ||
                    value != forbiddenCount[component]) {
                    heap.pop();
                } else {
                    break;
                }
            }

            if (heap.empty()) break;

            int value = heap.top().first;
            int component = heap.top().second;

            // Every current target is forbidden for this component.
            if (value >= targetVertexCount) break;

            heap.pop();

            if (constructEdges) {
                int sourceVertex = sink[component];
                int chosenTarget = -1;
                const vector<int> &blocked = forbiddenTargets[component];

                for (int target : targetVertices) {
                    if (!binary_search(blocked.begin(), blocked.end(), target)) {
                        chosenTarget = target;
                        break;
                    }
                }

                // value < targetVertexCount guarantees such a target exists.
                if (chosenTarget == -1) {
                    reached.clear();
                    return reached;
                }

                addedEdges.push_back({sourceVertex, chosenTarget});
            }

            addComponent(component);
        }

        return reached;
    };

    // Eliminate sets which cannot contain a universal reachable root.
    vector<char> active(componentCount, true);
    int activeCount = componentCount;
    int candidate = 0;

    while (true) {
        while (candidate < componentCount && !active[candidate]) {
            ++candidate;
        }

        vector<pair<int, int>> unused;
        vector<int> reached = runSearch(candidate, active, false, unused);

        if (static_cast<int>(reached.size()) == activeCount) {
            break;
        }

        for (int c : reached) {
            active[c] = false;
            --activeCount;
        }
    }

    // Verify the candidate on the entire component graph and construct edges.
    vector<char> allActive(componentCount, true);
    vector<pair<int, int>> addedEdges;
    vector<int> allReached =
        runSearch(candidate, allActive, true, addedEdges);

    if (static_cast<int>(allReached.size()) != componentCount ||
        static_cast<int>(addedEdges.size()) != componentCount - 1) {
        cout << "NO\n";
        return 0;
    }

    for (const auto &edge : mandatory) {
        cout << edge.first << ' ' << edge.second << '\n';
    }
    for (const auto &edge : addedEdges) {
        cout << edge.first << ' ' << edge.second << '\n';
    }

    return 0;
}