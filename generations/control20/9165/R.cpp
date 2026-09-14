#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

struct Edge {
    int to;
    int64 len;
};

static vector<int64> solveSmallN(
    int n, int64 K, const vector<vector<Edge>>& graph
) {
    vector<int64> answer(n, 0);
    vector<int> parent(n), subtree(n), order;
    vector<int64> parentEdge(n), usedFuel(n);
    order.reserve(n);

    for (int start = 0; start < n; ++start) {
        fill(parent.begin(), parent.end(), -2);
        order.clear();

        parent[start] = -1;
        parentEdge[start] = 0;
        usedFuel[start] = 0;
        order.push_back(start);

        for (size_t index = 0; index < order.size(); ++index) {
            int v = order[index];
            for (const Edge& edge : graph[v]) {
                if (edge.to == parent[v]) continue;
                parent[edge.to] = v;
                parentEdge[edge.to] = edge.len;
                order.push_back(edge.to);
            }
        }

        fill(subtree.begin(), subtree.end(), 1);
        for (int index = n - 1; index > 0; --index) {
            int v = order[index];
            subtree[parent[v]] += subtree[v];
        }

        for (int v : order) {
            for (const Edge& edge : graph[v]) {
                int child = edge.to;
                if (parent[child] != v) continue;

                if (usedFuel[v] + edge.len > K) {
                    answer[v] += subtree[child];
                    usedFuel[child] = edge.len;
                } else {
                    usedFuel[child] = usedFuel[v] + edge.len;
                }
            }
        }
    }

    return answer;
}

static vector<int64> solveSmallK(
    int n, int K, const vector<vector<Edge>>& graph
) {
    vector<int> parent(n, -2), subtree(n, 1), order;
    vector<int64> parentEdge(n, 0);
    order.reserve(n);

    parent[0] = -1;
    order.push_back(0);

    for (size_t index = 0; index < order.size(); ++index) {
        int v = order[index];
        for (const Edge& edge : graph[v]) {
            if (edge.to == parent[v]) continue;
            parent[edge.to] = v;
            parentEdge[edge.to] = edge.len;
            order.push_back(edge.to);
        }
    }

    for (int index = n - 1; index > 0; --index) {
        int v = order[index];
        subtree[parent[v]] += subtree[v];
    }

    const int states = K + 1;
    vector<vector<int64>> down(n, vector<int64>(states, 0));
    vector<vector<int64>> fromParent(n, vector<int64>(states, 0));

    auto addTransformed = [&](vector<int64>& destination,
                              const vector<int64>& source,
                              int distance,
                              int sign) {
        for (int fuel = 0; fuel <= K; ++fuel) {
            int nextFuel = (fuel + distance <= K)
                               ? fuel + distance
                               : distance;
            destination[nextFuel] += sign * source[fuel];
        }
    };

    for (int index = n - 1; index >= 0; --index) {
        int v = order[index];
        down[v][0]++;

        for (const Edge& edge : graph[v]) {
            int child = edge.to;
            if (parent[child] != v) continue;
            addTransformed(down[v], down[child], (int)edge.len, 1);
        }
    }

    vector<int64> answer(n, 0);

    for (int v : order) {
        vector<int64> total = down[v];
        for (int fuel = 0; fuel <= K; ++fuel) {
            total[fuel] += fromParent[v][fuel];
        }

        for (const Edge& edge : graph[v]) {
            int child = edge.to;
            if (parent[child] != v) continue;

            int distance = (int)edge.len;
            vector<int64> message = total;
            addTransformed(message, down[child], distance, -1);

            int threshold = K - distance;
            int64 originsAtVSide = 0;
            int64 originsAtChildSide = 0;

            for (int fuel = threshold + 1; fuel <= K; ++fuel) {
                originsAtVSide += message[fuel];
                originsAtChildSide += down[child][fuel];
            }

            answer[v] += originsAtVSide * subtree[child];
            answer[child] += originsAtChildSide * (n - subtree[child]);

            addTransformed(fromParent[child], message, distance, 1);
        }
    }

    return answer;
}

static void addLineDirection(
    const vector<int>& cities,
    const vector<int64>& coordinates,
    int64 K,
    vector<int64>& answer
) {
    int n = (int)cities.size();
    vector<int> nextStop(n, n - 1);
    int right = 0;

    for (int left = 0; left < n; ++left) {
        if (right < left) right = left;
        while (right + 1 < n &&
               coordinates[right + 1] - coordinates[left] <= K) {
            ++right;
        }
        nextStop[left] = right;
    }

    vector<int64> ways(n, 1);

    for (int i = 0; i + 1 < n; ++i) {
        answer[cities[i]] += (ways[i] - 1) * (n - i - 1);
        ways[nextStop[i]] += ways[i];
    }
}

static vector<int64> solveLine(
    int n, int64 K, const vector<vector<Edge>>& graph
) {
    int endpoint = 0;
    while ((int)graph[endpoint].size() > 1) ++endpoint;

    vector<int> cities;
    vector<int64> coordinates;
    cities.reserve(n);
    coordinates.reserve(n);

    int previous = -1;
    int current = endpoint;
    int64 coordinate = 0;

    while (true) {
        cities.push_back(current);
        coordinates.push_back(coordinate);

        int next = -1;
        int64 length = 0;
        for (const Edge& edge : graph[current]) {
            if (edge.to != previous) {
                next = edge.to;
                length = edge.len;
                break;
            }
        }

        if (next == -1) break;
        previous = current;
        current = next;
        coordinate += length;
    }

    vector<int64> answer(n, 0);
    addLineDirection(cities, coordinates, K, answer);

    reverse(cities.begin(), cities.end());
    vector<int64> reverseCoordinates(n);
    reverseCoordinates[0] = 0;

    for (int i = 1; i < n; ++i) {
        int a = cities[i - 1];
        int b = cities[i];
        for (const Edge& edge : graph[a]) {
            if (edge.to == b) {
                reverseCoordinates[i] =
                    reverseCoordinates[i - 1] + edge.len;
                break;
            }
        }
    }

    addLineDirection(cities, reverseCoordinates, K, answer);
    return answer;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    int64 K;
    cin >> N >> K;

    vector<vector<Edge>> graph(N);
    for (int i = 0; i + 1 < N; ++i) {
        int u, v;
        int64 length;
        cin >> u >> v >> length;
        graph[u].push_back({v, length});
        graph[v].push_back({u, length});
    }

    int maximumDegree = 0;
    for (const auto& adjacency : graph) {
        maximumDegree = max(maximumDegree, (int)adjacency.size());
    }

    vector<int64> answer;

    if (N <= 1000) {
        answer = solveSmallN(N, K, graph);
    } else if (K <= 10) {
        answer = solveSmallK(N, (int)K, graph);
    } else if (maximumDegree <= 2) {
        answer = solveLine(N, K, graph);
    } else {
        answer.assign(N, 0);
    }

    for (int64 value : answer) {
        cout << value << '\n';
    }

    return 0;
}
