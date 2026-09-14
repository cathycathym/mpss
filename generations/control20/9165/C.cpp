#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

struct AdjEdge {
    int to;
    int64 w;
    int arc; // directed arc: current vertex -> to
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    int64 K;
    cin >> n >> K;

    vector<vector<AdjEdge>> graph(n);
    vector<int64> arcWeight(2 * (n - 1));

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

    vector<int> parent(n, -2), parentArc(n, -1), order;
    vector<int64> subtree(n, 1);
    order.reserve(n);

    parent[0] = -1;
    vector<int> stackVertices = {0};

    while (!stackVertices.empty()) {
        int v = stackVertices.back();
        stackVertices.pop_back();
        order.push_back(v);

        for (const auto &e : graph[v]) {
            if (e.to == parent[v]) continue;
            parent[e.to] = v;
            parentArc[e.to] = e.arc ^ 1; // child -> parent
            stackVertices.push_back(e.to);
        }
    }

    for (int i = n - 1; i > 0; --i) {
        int v = order[i];
        subtree[parent[v]] += subtree[v];
    }

    auto sideSize = [&](int v, int u) -> int64 {
        if (parent[u] == v) return subtree[u];
        return n - subtree[v];
    };

    vector<int64> answer(n, 0);

    bool isPath = true;
    for (int v = 0; v < n; ++v) {
        if (graph[v].size() > 2) {
            isPath = false;
            break;
        }
    }

    if (isPath) {
        int endpoint = 0;
        while (graph[endpoint].size() != 1) ++endpoint;

        vector<int> path;
        vector<int64> edgeLength;
        path.reserve(n);
        edgeLength.reserve(n - 1);

        int previous = -1;
        int current = endpoint;

        while (true) {
            path.push_back(current);
            int next = -1;
            int64 nextWeight = 0;

            for (const auto &e : graph[current]) {
                if (e.to != previous) {
                    next = e.to;
                    nextWeight = e.w;
                    break;
                }
            }

            if (next == -1) break;
            edgeLength.push_back(nextWeight);
            previous = current;
            current = next;
        }

        vector<int64> prefix(n, 0);
        for (int i = 0; i + 1 < n; ++i) {
            prefix[i + 1] = prefix[i] + edgeLength[i];
        }

        // Trips from left to right.
        vector<int64> mass(n, 1);
        int farthest = 0;

        for (int x = 0; x + 1 < n; ++x) {
            farthest = max(farthest, x);
            while (farthest + 1 < n &&
                   prefix[farthest + 1] - prefix[x] <= K) {
                ++farthest;
            }

            if (farthest < n - 1) {
                int stop = farthest;
                answer[path[stop]] +=
                    mass[x] * static_cast<int64>(n - 1 - stop);
                mass[stop] += mass[x];
            }
        }

        // Trips from right to left.
        fill(mass.begin(), mass.end(), 1);
        farthest = n - 1;

        for (int x = n - 1; x > 0; --x) {
            farthest = min(farthest, x);
            while (farthest - 1 >= 0 &&
                   prefix[x] - prefix[farthest - 1] <= K) {
                --farthest;
            }

            if (farthest > 0) {
                int stop = farthest;
                answer[path[stop]] += mass[x] * static_cast<int64>(stop);
                mass[stop] += mass[x];
            }
        }
    } else if (K <= 10) {
        int states = static_cast<int>(K) + 1;
        int arcs = 2 * (n - 1);

        vector<vector<int64>> message(
            arcs, vector<int64>(states, 0)
        );

        auto transform = [&](const vector<int64> &source,
                             vector<int64> &destination,
                             int64 w) {
            fill(destination.begin(), destination.end(), 0);
            for (int d = 0; d < states; ++d) {
                if (source[d] == 0) continue;
                if (static_cast<int64>(d) + w > K) {
                    destination[static_cast<int>(w)] += source[d];
                } else {
                    destination[static_cast<int>(d + w)] += source[d];
                }
            }
        };

        // Bottom-up messages: child -> parent.
        for (int index = n - 1; index > 0; --index) {
            int v = order[index];
            vector<int64> combined(states, 0);
            combined[0] = 1; // starting city v

            for (const auto &e : graph[v]) {
                if (parent[e.to] == v) {
                    int incomingArc = e.arc ^ 1;
                    for (int d = 0; d < states; ++d) {
                        combined[d] += message[incomingArc][d];
                    }
                }
            }

            transform(combined, message[parentArc[v]],
                      arcWeight[parentArc[v]]);
        }

        // Top-down rerooting. Parent -> child messages are available
        // before each child is processed.
        for (int v : order) {
            vector<int64> total(states, 0);
            total[0] = 1;

            for (const auto &e : graph[v]) {
                int incomingArc = e.arc ^ 1;
                for (int d = 0; d < states; ++d) {
                    total[d] += message[incomingArc][d];
                }
            }

            for (const auto &e : graph[v]) {
                int incomingArc = e.arc ^ 1;
                vector<int64> cavity(states, 0);

                int64 refillStarts = 0;
                for (int d = 0; d < states; ++d) {
                    cavity[d] = total[d] - message[incomingArc][d];
                    if (static_cast<int64>(d) + e.w > K) {
                        refillStarts += cavity[d];
                    }
                }

                answer[v] += refillStarts * sideSize(v, e.to);
                transform(cavity, message[e.arc], e.w);
            }
        }
    } else {
        // Exact quadratic fallback.
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

                int v = state.vertex;
                for (const auto &e : graph[v]) {
                    if (e.to == state.previous) continue;

                    bool refuel = state.consumed + e.w > K;
                    int64 nextConsumed;

                    if (refuel) {
                        answer[v] += sideSize(v, e.to);
                        nextConsumed = e.w;
                    } else {
                        nextConsumed = state.consumed + e.w;
                    }

                    traversal.push_back({e.to, v, nextConsumed});
                }
            }
        }
    }

    for (int v = 0; v < n; ++v) {
        cout << answer[v] << '\n';
    }

    return 0;
}
