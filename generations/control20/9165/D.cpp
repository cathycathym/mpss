#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

struct Edge {
    int to;
    int64 weight;
    int arc;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    int64 K;
    cin >> n >> K;

    const int arcCount = 2 * (n - 1);
    vector<vector<Edge>> graph(n);
    vector<int64> arcWeight(arcCount);

    for (int i = 0; i < n - 1; ++i) {
        int u, v;
        int64 w;
        cin >> u >> v >> w;

        int a = 2 * i;
        int b = a ^ 1;
        graph[u].push_back({v, w, a});
        graph[v].push_back({u, w, b});
        arcWeight[a] = arcWeight[b] = w;
    }

    vector<int> parent(n, -1), parentArc(n, -1), order;
    vector<int64> subtree(n, 1);
    order.reserve(n);

    parent[0] = 0;
    vector<int> stack = {0};

    while (!stack.empty()) {
        int v = stack.back();
        stack.pop_back();
        order.push_back(v);

        for (const Edge& edge : graph[v]) {
            if (edge.to == parent[v]) continue;
            parent[edge.to] = v;
            parentArc[edge.to] = edge.arc ^ 1;
            stack.push_back(edge.to);
        }
    }

    for (int i = n - 1; i > 0; --i) {
        int v = order[i];
        subtree[parent[v]] += subtree[v];
    }

    auto sideSize = [&](int v, int neighbor) -> int64 {
        if (parent[neighbor] == v) return subtree[neighbor];
        return n - subtree[v];
    };

    vector<int64> answer(n, 0);

    bool isPath = true;
    int starCenter = -1;

    for (int v = 0; v < n; ++v) {
        if (graph[v].size() > 2) isPath = false;
        if (static_cast<int>(graph[v].size()) == n - 1) starCenter = v;
    }

    if (isPath) {
        int endpoint = 0;
        while (graph[endpoint].size() != 1) ++endpoint;

        vector<int> path;
        vector<int64> lengths;
        path.reserve(n);
        lengths.reserve(n - 1);

        int previous = -1;
        int current = endpoint;

        while (true) {
            path.push_back(current);

            int next = -1;
            int64 nextLength = 0;

            for (const Edge& edge : graph[current]) {
                if (edge.to != previous) {
                    next = edge.to;
                    nextLength = edge.weight;
                    break;
                }
            }

            if (next == -1) break;

            lengths.push_back(nextLength);
            previous = current;
            current = next;
        }

        vector<int64> prefix(n, 0);
        for (int i = 0; i + 1 < n; ++i) {
            prefix[i + 1] = prefix[i] + lengths[i];
        }

        vector<int64> mass(n, 1);
        int farthest = 0;

        for (int start = 0; start + 1 < n; ++start) {
            farthest = max(farthest, start);

            while (farthest + 1 < n &&
                   prefix[farthest + 1] - prefix[start] <= K) {
                ++farthest;
            }

            if (farthest + 1 < n) {
                answer[path[farthest]] +=
                    mass[start] * static_cast<int64>(n - 1 - farthest);
                mass[farthest] += mass[start];
            }
        }

        fill(mass.begin(), mass.end(), 1);
        farthest = n - 1;

        for (int start = n - 1; start > 0; --start) {
            farthest = min(farthest, start);

            while (farthest > 0 &&
                   prefix[start] - prefix[farthest - 1] <= K) {
                --farthest;
            }

            if (farthest > 0) {
                answer[path[farthest]] +=
                    mass[start] * static_cast<int64>(farthest);
                mass[farthest] += mass[start];
            }
        }
    } else if (starCenter != -1) {
        vector<int64> weights;
        weights.reserve(n - 1);

        for (const Edge& edge : graph[starCenter]) {
            weights.push_back(edge.weight);
        }

        vector<int64> sorted = weights;
        sort(sorted.begin(), sorted.end());

        for (int64 destinationWeight : weights) {
            int64 threshold = K - destinationWeight;
            auto first = upper_bound(sorted.begin(), sorted.end(), threshold);
            int64 count = sorted.end() - first;

            if (destinationWeight > threshold) {
                --count;
            }

            answer[starCenter] += count;
        }
    } else {
        const int64 maxCells = 25000000LL;
        bool useStateDP =
            K < INT_MAX &&
            static_cast<int64>(arcCount) * (K + 1) <= maxCells;

        if (useStateDP) {
            int states = static_cast<int>(K) + 1;
            vector<int> message(static_cast<size_t>(arcCount) * states, 0);

            auto cell = [&](int arc, int state) -> int& {
                return message[static_cast<size_t>(arc) * states + state];
            };

            vector<int> total(states), cavity(states), transformed(states);

            auto transform = [&](const vector<int>& source,
                                 int outputArc,
                                 int64 weight) {
                fill(transformed.begin(), transformed.end(), 0);

                for (int state = 0; state < states; ++state) {
                    int count = source[state];
                    if (count == 0) continue;

                    int nextState;
                    if (static_cast<int64>(state) + weight > K) {
                        nextState = static_cast<int>(weight);
                    } else {
                        nextState = static_cast<int>(state + weight);
                    }

                    transformed[nextState] += count;
                }

                for (int state = 0; state < states; ++state) {
                    cell(outputArc, state) = transformed[state];
                }
            };

            for (int index = n - 1; index > 0; --index) {
                int v = order[index];
                fill(total.begin(), total.end(), 0);
                total[0] = 1;

                for (const Edge& edge : graph[v]) {
                    if (parent[edge.to] != v) continue;

                    int incomingArc = edge.arc ^ 1;
                    for (int state = 0; state < states; ++state) {
                        total[state] += cell(incomingArc, state);
                    }
                }

                transform(total, parentArc[v], arcWeight[parentArc[v]]);
            }

            for (int v : order) {
                fill(total.begin(), total.end(), 0);
                total[0] = 1;

                for (const Edge& edge : graph[v]) {
                    int incomingArc = edge.arc ^ 1;
                    for (int state = 0; state < states; ++state) {
                        total[state] += cell(incomingArc, state);
                    }
                }

                for (const Edge& edge : graph[v]) {
                    int incomingArc = edge.arc ^ 1;
                    int64 refillSources = 0;

                    for (int state = 0; state < states; ++state) {
                        cavity[state] = total[state] - cell(incomingArc, state);

                        if (static_cast<int64>(state) + edge.weight > K) {
                            refillSources += cavity[state];
                        }
                    }

                    answer[v] += refillSources * sideSize(v, edge.to);
                    transform(cavity, edge.arc, edge.weight);
                }
            }
        } else {
            struct State {
                int vertex;
                int previous;
                int64 consumed;
            };

            vector<State> traversal;
            traversal.reserve(n);

            for (int source = 0; source < n; ++source) {
                traversal.clear();
                traversal.push_back({source, -1, 0});

                while (!traversal.empty()) {
                    State state = traversal.back();
                    traversal.pop_back();

                    for (const Edge& edge : graph[state.vertex]) {
                        if (edge.to == state.previous) continue;

                        int64 nextConsumed;

                        if (state.consumed + edge.weight > K) {
                            answer[state.vertex] +=
                                sideSize(state.vertex, edge.to);
                            nextConsumed = edge.weight;
                        } else {
                            nextConsumed = state.consumed + edge.weight;
                        }

                        traversal.push_back(
                            {edge.to, state.vertex, nextConsumed}
                        );
                    }
                }
            }
        }
    }

    for (int v = 0; v < n; ++v) {
        cout << answer[v] << '\n';
    }

    return 0;
}
