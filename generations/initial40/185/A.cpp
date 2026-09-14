#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <numeric>
#include <cstdint>

using namespace std;

struct Edge {
    int u, v;
    long long limit;
};

struct Query {
    int type;
    int x;
    long long y;
};

class RollbackDSU {
private:
    struct Change {
        int child;
        int parentSizeBefore;
    };

    vector<int> parent;
    vector<int> componentSize;
    vector<Change> history;

public:
    explicit RollbackDSU(int n) {
        parent.resize(n + 1);
        componentSize.assign(n + 1, 1);
        iota(parent.begin(), parent.end(), 0);
        history.reserve(n);
    }

    int find(int vertex) const {
        while (parent[vertex] != vertex) {
            vertex = parent[vertex];
        }
        return vertex;
    }

    void unite(int a, int b) {
        a = find(a);
        b = find(b);

        if (a == b) {
            return;
        }

        if (componentSize[a] < componentSize[b]) {
            swap(a, b);
        }

        history.push_back({b, componentSize[a]});
        parent[b] = a;
        componentSize[a] += componentSize[b];
    }

    int snapshot() const {
        return static_cast<int>(history.size());
    }

    void rollback(int snapshotSize) {
        while (static_cast<int>(history.size()) > snapshotSize) {
            Change change = history.back();
            history.pop_back();

            int child = change.child;
            int root = parent[child];

            componentSize[root] = change.parentSizeBefore;
            parent[child] = child;
        }
    }

    int sizeOf(int vertex) const {
        return componentSize[find(vertex)];
    }
};

struct BlockRequest {
    int start;
    long long weight;
    int answerIndex;
    vector<long long> specialLimits;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    vector<Edge> edges(m);
    vector<long long> currentLimit(m);

    set<pair<long long, int>, greater<pair<long long, int>>> orderedEdges;

    for (int i = 0; i < m; ++i) {
        cin >> edges[i].u >> edges[i].v >> edges[i].limit;
        currentLimit[i] = edges[i].limit;
        orderedEdges.insert({currentLimit[i], i});
    }

    int q;
    cin >> q;

    vector<Query> queries(q);
    int answerCount = 0;

    for (int i = 0; i < q; ++i) {
        cin >> queries[i].type >> queries[i].x >> queries[i].y;
        --queries[i].x;

        if (queries[i].type == 2) {
            ++queries[i].x;
            ++answerCount;
        }
    }

    vector<int> answers(answerCount);
    vector<int> specialPosition(m, -1);

    const int BLOCK_SIZE = 400;
    int nextAnswerIndex = 0;

    for (int blockLeft = 0; blockLeft < q; blockLeft += BLOCK_SIZE) {
        int blockRight = min(q, blockLeft + BLOCK_SIZE);

        vector<int> specialEdges;
        specialEdges.reserve(BLOCK_SIZE);

        for (int i = blockLeft; i < blockRight; ++i) {
            if (queries[i].type == 1) {
                int edgeId = queries[i].x;
                if (specialPosition[edgeId] == -1) {
                    specialPosition[edgeId] =
                        static_cast<int>(specialEdges.size());
                    specialEdges.push_back(edgeId);
                }
            }
        }

        vector<long long> blockLimits(specialEdges.size());
        for (int i = 0; i < static_cast<int>(specialEdges.size()); ++i) {
            blockLimits[i] = currentLimit[specialEdges[i]];
        }

        vector<BlockRequest> requests;
        requests.reserve(BLOCK_SIZE);

        for (int i = blockLeft; i < blockRight; ++i) {
            if (queries[i].type == 1) {
                int edgeId = queries[i].x;
                blockLimits[specialPosition[edgeId]] = queries[i].y;
            } else {
                BlockRequest request;
                request.start = queries[i].x;
                request.weight = queries[i].y;
                request.answerIndex = nextAnswerIndex++;
                request.specialLimits = blockLimits;
                requests.push_back(std::move(request));
            }
        }

        vector<int> stableEdges;
        stableEdges.reserve(m - static_cast<int>(specialEdges.size()));

        for (const auto& entry : orderedEdges) {
            int edgeId = entry.second;
            if (specialPosition[edgeId] == -1) {
                stableEdges.push_back(edgeId);
            }
        }

        sort(requests.begin(), requests.end(),
             [](const BlockRequest& a, const BlockRequest& b) {
                 return a.weight > b.weight;
             });

        RollbackDSU dsu(n);
        int stablePointer = 0;

        for (const BlockRequest& request : requests) {
            while (stablePointer < static_cast<int>(stableEdges.size()) &&
                   currentLimit[stableEdges[stablePointer]] >= request.weight) {
                int edgeId = stableEdges[stablePointer++];
                dsu.unite(edges[edgeId].u, edges[edgeId].v);
            }

            int savedState = dsu.snapshot();

            for (int i = 0; i < static_cast<int>(specialEdges.size()); ++i) {
                if (request.specialLimits[i] >= request.weight) {
                    int edgeId = specialEdges[i];
                    dsu.unite(edges[edgeId].u, edges[edgeId].v);
                }
            }

            answers[request.answerIndex] = dsu.sizeOf(request.start);
            dsu.rollback(savedState);
        }

        for (int i = 0; i < static_cast<int>(specialEdges.size()); ++i) {
            int edgeId = specialEdges[i];

            orderedEdges.erase({currentLimit[edgeId], edgeId});
            currentLimit[edgeId] = blockLimits[i];
            edges[edgeId].limit = blockLimits[i];
            orderedEdges.insert({currentLimit[edgeId], edgeId});

            specialPosition[edgeId] = -1;
        }
    }

    for (int answer : answers) {
        cout << answer << '\n';
    }

    return 0;
}