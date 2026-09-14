#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <utility>
#include <cstdint>

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

static void exploreReverseGraph(
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
        int target = queue[head];
        ++timestamp;

        for (int blockedSource : blockedIncoming[target]) {
            mark[blockedSource] = timestamp;
        }

        for (auto it = unvisited.begin(); it != unvisited.end();) {
            int source = *it;

            if (mark[source] != timestamp) {
                it = unvisited.erase(it);
                queue.push_back(source);

                if (treeParent != nullptr) {
                    (*treeParent)[source] = target;
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

    vector<pair<int, int>> mandatoryEdges;
    mandatoryEdges.reserve(M);

    vector<int> mandatoryParent(N, -1);
    DSU dsu(N);
    bool valid = true;

    for (int i = 0; i < M; ++i) {
        int from, to;
        cin >> from >> to;
        mandatoryEdges.push_back({from, to});

        if (mandatoryParent[from] != -1) {
            valid = false;
        } else {
            mandatoryParent[from] = to;
        }

        if (!dsu.unite(from, to)) {
            valid = false;
        }
    }

    vector<pair<int, int>> forbiddenEdges(K);
    for (auto& edge : forbiddenEdges) {
        cin >> edge.first >> edge.second;
    }

    if (!valid) {
        cout << "NO\n";
        return 0;
    }

    vector<int> leaderToComponent(N, -1);
    vector<int> componentOf(N);
    int componentCount = 0;

    for (int vertex = 0; vertex < N; ++vertex) {
        int leader = dsu.find(vertex);

        if (leaderToComponent[leader] == -1) {
            leaderToComponent[leader] = componentCount++;
        }

        componentOf[vertex] = leaderToComponent[leader];
    }

    vector<int> componentTop(componentCount, -1);
    vector<int> componentSize(componentCount, 0);
    vector<vector<int>> componentVertices(componentCount);

    for (int vertex = 0; vertex < N; ++vertex) {
        int component = componentOf[vertex];

        ++componentSize[component];
        componentVertices[component].push_back(vertex);

        if (mandatoryParent[vertex] == -1) {
            if (componentTop[component] != -1) {
                valid = false;
            }
            componentTop[component] = vertex;
        }
    }

    for (int component = 0; component < componentCount; ++component) {
        if (componentTop[component] == -1) {
            valid = false;
        }
    }

    if (!valid) {
        cout << "NO\n";
        return 0;
    }

    using U64 = uint64_t;

    vector<U64> allForbidden;
    allForbidden.reserve(K);

    vector<U64> contractedForbidden;
    contractedForbidden.reserve(K);

    for (const auto& edge : forbiddenEdges) {
        int from = edge.first;
        int to = edge.second;

        allForbidden.push_back(
            static_cast<U64>(from) * static_cast<U64>(N) +
            static_cast<U64>(to)
        );

        int sourceComponent = componentOf[from];
        int targetComponent = componentOf[to];

        if (sourceComponent != targetComponent &&
            componentTop[sourceComponent] == from) {
            contractedForbidden.push_back(
                static_cast<U64>(sourceComponent) *
                    static_cast<U64>(componentCount) +
                static_cast<U64>(targetComponent)
            );
        }
    }

    sort(allForbidden.begin(), allForbidden.end());
    sort(contractedForbidden.begin(), contractedForbidden.end());

    vector<vector<int>> blockedIncoming(componentCount);

    for (size_t left = 0; left < contractedForbidden.size();) {
        size_t right = left + 1;

        while (right < contractedForbidden.size() &&
               contractedForbidden[right] == contractedForbidden[left]) {
            ++right;
        }

        U64 key = contractedForbidden[left];
        int sourceComponent =
            static_cast<int>(key / static_cast<U64>(componentCount));
        int targetComponent =
            static_cast<int>(key % static_cast<U64>(componentCount));

        if (static_cast<int>(right - left) ==
            componentSize[targetComponent]) {
            blockedIncoming[targetComponent].push_back(sourceComponent);
        }

        left = right;
    }

    vector<int> mark(componentCount, 0);
    int timestamp = 0;

    list<int> unvisited;
    for (int component = 0; component < componentCount; ++component) {
        unvisited.push_back(component);
    }

    int candidate = -1;

    while (!unvisited.empty()) {
        int start = unvisited.front();
        unvisited.pop_front();
        candidate = start;

        exploreReverseGraph(
            start,
            unvisited,
            blockedIncoming,
            mark,
            timestamp,
            nullptr
        );
    }

    unvisited.clear();
    for (int component = 0; component < componentCount; ++component) {
        if (component != candidate) {
            unvisited.push_back(component);
        }
    }

    vector<int> componentParent(componentCount, -2);
    componentParent[candidate] = -1;

    exploreReverseGraph(
        candidate,
        unvisited,
        blockedIncoming,
        mark,
        timestamp,
        &componentParent
    );

    if (!unvisited.empty()) {
        cout << "NO\n";
        return 0;
    }

    vector<pair<int, int>> answer = mandatoryEdges;
    answer.reserve(N - 1);

    for (int sourceComponent = 0;
         sourceComponent < componentCount;
         ++sourceComponent) {
        if (sourceComponent == candidate) continue;

        int targetComponent = componentParent[sourceComponent];
        int sourceVertex = componentTop[sourceComponent];
        int targetVertex = -1;

        for (int vertex : componentVertices[targetComponent]) {
            U64 key =
                static_cast<U64>(sourceVertex) * static_cast<U64>(N) +
                static_cast<U64>(vertex);

            if (!binary_search(allForbidden.begin(),
                               allForbidden.end(), key)) {
                targetVertex = vertex;
                break;
            }
        }

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