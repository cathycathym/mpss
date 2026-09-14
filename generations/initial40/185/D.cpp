#include <algorithm>
#include <iostream>
#include <numeric>
#include <set>
#include <utility>
#include <vector>

using namespace std;

class DSU {
public:
    vector<int> parent;
    vector<int> componentSize;

    explicit DSU(int n = 0) {
        reset(n);
    }

    void reset(int n) {
        parent.resize(n);
        componentSize.assign(n, 1);
        iota(parent.begin(), parent.end(), 0);
    }

    int find(int vertex) {
        int root = vertex;
        while (parent[root] != root) {
            root = parent[root];
        }

        while (parent[vertex] != vertex) {
            int next = parent[vertex];
            parent[vertex] = root;
            vertex = next;
        }

        return root;
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

        parent[b] = a;
        componentSize[a] += componentSize[b];
    }
};

struct Operation {
    int type;
    int argument;
    long long value;
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

    for (int edge = 0; edge < m; ++edge) {
        cin >> edgeU[edge] >> edgeV[edge] >> edgeWeight[edge];
        --edgeU[edge];
        --edgeV[edge];
        orderedEdges.insert({edgeWeight[edge], edge});
    }

    int q;
    cin >> q;

    vector<Operation> operations(q);
    int answerCount = 0;

    for (int i = 0; i < q; ++i) {
        int type, argument;
        long long value;
        cin >> type >> argument >> value;
        --argument;

        operations[i] = {type, argument, value, -1};
        if (type == 2) {
            operations[i].answerIndex = answerCount++;
        }
    }

    vector<int> answers(answerCount);

    const int BLOCK_SIZE = 700;

    vector<int> volatilePosition(m, -1);
    vector<int> componentIndex(n);
    vector<int> componentStamp(n, 0);
    int currentStamp = 0;

    DSU staticDSU(n);

    for (int blockLeft = 0; blockLeft < q; blockLeft += BLOCK_SIZE) {
        int blockRight = min(q, blockLeft + BLOCK_SIZE);

        vector<int> volatileEdges;
        volatileEdges.reserve(blockRight - blockLeft);

        for (int i = blockLeft; i < blockRight; ++i) {
            if (operations[i].type == 1) {
                int edge = operations[i].argument;
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
            const Operation& operation = operations[i];

            if (operation.type == 1) {
                currentVolatileWeight[volatilePosition[operation.argument]] =
                    operation.value;
            } else {
                int stateIndex = static_cast<int>(blockQueries.size());
                blockQueries.push_back({
                    operation.argument,
                    operation.value,
                    operation.answerIndex,
                    stateIndex
                });

                savedStates.insert(
                    savedStates.end(),
                    currentVolatileWeight.begin(),
                    currentVolatileWeight.end()
                );
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
        vector<int> localSize(2 * volatileCount + 1);

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

            ++currentStamp;
            int localCount = 0;

            auto addComponent = [&](int root) {
                if (componentStamp[root] != currentStamp) {
                    componentStamp[root] = currentStamp;
                    componentIndex[root] = localCount;
                    localParent[localCount] = localCount;
                    localSize[localCount] = staticDSU.componentSize[root];
                    ++localCount;
                }
                return componentIndex[root];
            };

            auto localFind = [&](int vertex) {
                int root = vertex;
                while (localParent[root] != root) {
                    root = localParent[root];
                }

                while (localParent[vertex] != vertex) {
                    int next = localParent[vertex];
                    localParent[vertex] = root;
                    vertex = next;
                }

                return root;
            };

            int sourceComponent = staticDSU.find(query.vertex);
            int sourceLocal = addComponent(sourceComponent);

            size_t stateOffset =
                static_cast<size_t>(query.stateIndex) * volatileCount;

            for (int i = 0; i < volatileCount; ++i) {
                if (savedStates[stateOffset + i] < query.weight) {
                    continue;
                }

                int edge = volatileEdges[i];
                int componentA = staticDSU.find(edgeU[edge]);
                int componentB = staticDSU.find(edgeV[edge]);

                int localA = localFind(addComponent(componentA));
                int localB = localFind(addComponent(componentB));

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
            const Operation& operation = operations[i];

            if (operation.type == 1) {
                int edge = operation.argument;
                orderedEdges.erase({edgeWeight[edge], edge});
                edgeWeight[edge] = operation.value;
                orderedEdges.insert({edgeWeight[edge], edge});
            }
        }

        for (int edge : volatileEdges) {
            volatilePosition[edge] = -1;
        }
    }

    for (int answer : answers) {
        cout << answer << '\n';
    }

    return 0;
}