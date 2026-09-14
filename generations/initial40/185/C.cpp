#include <algorithm>
#include <iostream>
#include <numeric>
#include <set>
#include <utility>
#include <vector>

using namespace std;

struct DSU {
    vector<int> parent;
    vector<int> size;

    DSU() = default;
    explicit DSU(int n) {
        reset(n);
    }

    void reset(int n) {
        parent.resize(n);
        size.assign(n, 1);
        iota(parent.begin(), parent.end(), 0);
    }

    int find(int v) {
        int root = v;
        while (parent[root] != root) {
            root = parent[root];
        }
        while (parent[v] != v) {
            int next = parent[v];
            parent[v] = root;
            v = next;
        }
        return root;
    }

    void unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) {
            return;
        }
        if (size[a] < size[b]) {
            swap(a, b);
        }
        parent[b] = a;
        size[a] += size[b];
    }

    int componentSize(int v) {
        return size[find(v)];
    }
};

struct Operation {
    int type;
    int a;
    long long x;
    int answerIndex;
};

struct BlockQuery {
    int vertex;
    long long weight;
    int answerIndex;
    int stateIndex;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    vector<int> edgeU(m), edgeV(m);
    vector<long long> edgeWeight(m);
    set<pair<long long, int>> orderedEdges;

    for (int i = 0; i < m; ++i) {
        cin >> edgeU[i] >> edgeV[i] >> edgeWeight[i];
        --edgeU[i];
        --edgeV[i];
        orderedEdges.insert({edgeWeight[i], i});
    }

    int q;
    cin >> q;

    vector<Operation> operations(q);
    int answerCount = 0;

    for (int i = 0; i < q; ++i) {
        int type, a;
        long long x;
        cin >> type >> a >> x;
        --a;

        operations[i] = {type, a, x, -1};
        if (type == 2) {
            operations[i].answerIndex = answerCount++;
        }
    }

    vector<long long> answers(answerCount);

    const int BLOCK_SIZE = 700;

    vector<int> volatilePosition(m, -1);
    vector<int> componentLocalIndex(n, 0);
    vector<int> componentSeen(n, 0);
    int stamp = 0;

    DSU staticDSU(n);

    for (int blockLeft = 0; blockLeft < q; blockLeft += BLOCK_SIZE) {
        int blockRight = min(q, blockLeft + BLOCK_SIZE);

        vector<int> volatileEdges;
        volatileEdges.reserve(blockRight - blockLeft);

        for (int i = blockLeft; i < blockRight; ++i) {
            if (operations[i].type == 1) {
                int edge = operations[i].a;
                if (volatilePosition[edge] == -1) {
                    volatilePosition[edge] =
                        static_cast<int>(volatileEdges.size());
                    volatileEdges.push_back(edge);
                }
            }
        }

        int volatileCount = static_cast<int>(volatileEdges.size());

        vector<long long> currentVolatileWeight(volatileCount);
        for (int i = 0; i < volatileCount; ++i) {
            currentVolatileWeight[i] = edgeWeight[volatileEdges[i]];
        }

        vector<BlockQuery> blockQueries;
        blockQueries.reserve(blockRight - blockLeft);

        vector<long long> savedStates;
        savedStates.reserve(
            static_cast<size_t>(blockRight - blockLeft) * volatileCount
        );

        for (int i = blockLeft; i < blockRight; ++i) {
            const Operation& op = operations[i];

            if (op.type == 1) {
                currentVolatileWeight[volatilePosition[op.a]] = op.x;
            } else {
                int stateIndex = static_cast<int>(blockQueries.size());
                blockQueries.push_back(
                    {op.a, op.x, op.answerIndex, stateIndex}
                );
                for (int j = 0; j < volatileCount; ++j) {
                    savedStates.push_back(currentVolatileWeight[j]);
                }
            }
        }

        vector<int> queryOrder(blockQueries.size());
        iota(queryOrder.begin(), queryOrder.end(), 0);

        sort(queryOrder.begin(), queryOrder.end(),
             [&](int lhs, int rhs) {
                 return blockQueries[lhs].weight >
                        blockQueries[rhs].weight;
             });

        staticDSU.reset(n);
        auto edgeIterator = orderedEdges.rbegin();

        vector<int> localParent(2 * volatileCount + 1);
        vector<long long> localSize(2 * volatileCount + 1);

        for (int queryIndex : queryOrder) {
            const BlockQuery& query = blockQueries[queryIndex];

            while (edgeIterator != orderedEdges.rend() &&
                   edgeIterator->first >= query.weight) {
                int edge = edgeIterator->second;
                if (volatilePosition[edge] == -1) {
                    staticDSU.unite(edgeU[edge], edgeV[edge]);
                }
                ++edgeIterator;
            }

            ++stamp;
            int localCount = 0;

            auto addStaticComponent = [&](int root) {
                if (componentSeen[root] != stamp) {
                    componentSeen[root] = stamp;
                    componentLocalIndex[root] = localCount;
                    localParent[localCount] = localCount;
                    localSize[localCount] = staticDSU.size[root];
                    ++localCount;
                }
                return componentLocalIndex[root];
            };

            auto localFind = [&](int v) {
                int root = v;
                while (localParent[root] != root) {
                    root = localParent[root];
                }
                while (localParent[v] != v) {
                    int next = localParent[v];
                    localParent[v] = root;
                    v = next;
                }
                return root;
            };

            int sourceRoot = staticDSU.find(query.vertex);
            int sourceLocal = addStaticComponent(sourceRoot);

            size_t stateBase =
                static_cast<size_t>(query.stateIndex) * volatileCount;

            for (int j = 0; j < volatileCount; ++j) {
                if (savedStates[stateBase + j] < query.weight) {
                    continue;
                }

                int edge = volatileEdges[j];
                int rootA = staticDSU.find(edgeU[edge]);
                int rootB = staticDSU.find(edgeV[edge]);

                int localA = addStaticComponent(rootA);
                int localB = addStaticComponent(rootB);

                localA = localFind(localA);
                localB = localFind(localB);

                if (localA == localB) {
                    continue;
                }

                if (localSize[localA] < localSize[localB]) {
                    swap(localA, localB);
                }

                localParent[localB] = localA;
                localSize[localA] += localSize[localB];
            }

            answers[query.answerIndex] =
                localSize[localFind(sourceLocal)];
        }

        for (int i = blockLeft; i < blockRight; ++i) {
            const Operation& op = operations[i];
            if (op.type == 1) {
                int edge = op.a;
                orderedEdges.erase({edgeWeight[edge], edge});
                edgeWeight[edge] = op.x;
                orderedEdges.insert({edgeWeight[edge], edge});
            }
        }

        for (int edge : volatileEdges) {
            volatilePosition[edge] = -1;
        }
    }

    for (long long answer : answers) {
        cout << answer << '\n';
    }

    return 0;
}