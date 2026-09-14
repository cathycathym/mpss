#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

struct Edge {
    int to;
    int len;
};

struct SegmentTree {
    int n;
    vector<int64> sum;
    vector<unsigned char> cleared;

    explicit SegmentTree(int size) : n(size), sum(4 * size), cleared(4 * size, 0) {}

    void applyClear(int node) {
        sum[node] = 0;
        cleared[node] = 1;
    }

    void push(int node) {
        if (!cleared[node]) return;
        applyClear(node * 2);
        applyClear(node * 2 + 1);
        cleared[node] = 0;
    }

    void add(int node, int left, int right, int pos, int64 value) {
        if (left == right) {
            sum[node] += value;
            cleared[node] = 0;
            return;
        }
        push(node);
        int mid = (left + right) / 2;
        if (pos <= mid) add(node * 2, left, mid, pos, value);
        else add(node * 2 + 1, mid + 1, right, pos, value);
        sum[node] = sum[node * 2] + sum[node * 2 + 1];
    }

    void add(int pos, int64 value) {
        add(1, 0, n - 1, pos, value);
    }

    int64 query(int node, int left, int right, int ql, int qr) {
        if (ql <= left && right <= qr) return sum[node];
        push(node);
        int mid = (left + right) / 2;
        int64 result = 0;
        if (ql <= mid) result += query(node * 2, left, mid, ql, qr);
        if (qr > mid) result += query(node * 2 + 1, mid + 1, right, ql, qr);
        return result;
    }

    int64 query(int left, int right) {
        if (left > right) return 0;
        return query(1, 0, n - 1, left, right);
    }

    void clearRange(int node, int left, int right, int ql, int qr) {
        if (ql <= left && right <= qr) {
            applyClear(node);
            return;
        }
        push(node);
        int mid = (left + right) / 2;
        if (ql <= mid) clearRange(node * 2, left, mid, ql, qr);
        if (qr > mid) clearRange(node * 2 + 1, mid + 1, right, ql, qr);
        sum[node] = sum[node * 2] + sum[node * 2 + 1];
    }

    void clearRange(int left, int right) {
        if (left <= right) clearRange(1, 0, n - 1, left, right);
    }
};

static void solvePath(const vector<int>& vertices,
                      const vector<int>& lengths,
                      int64 K,
                      vector<int64>& answer) {
    int n = (int)vertices.size();
    vector<int64> prefix(n, 0);
    for (int i = 0; i + 1 < n; ++i)
        prefix[i + 1] = prefix[i] + lengths[i];

    vector<int64> coordinates;
    coordinates.reserve(n);
    for (int i = 0; i < n; ++i)
        coordinates.push_back(-prefix[i]);

    sort(coordinates.begin(), coordinates.end());
    coordinates.erase(unique(coordinates.begin(), coordinates.end()),
                      coordinates.end());

    SegmentTree tree((int)coordinates.size());

    auto position = [&](int64 value) {
        return (int)(lower_bound(coordinates.begin(), coordinates.end(), value)
                     - coordinates.begin());
    };

    tree.add(position(0), 1);

    for (int i = 0; i + 1 < n; ++i) {
        int64 threshold = K - prefix[i + 1];
        int firstHigh = (int)(upper_bound(coordinates.begin(),
                                         coordinates.end(),
                                         threshold)
                              - coordinates.begin());

        int64 mustRefuel = 0;
        if (firstHigh < (int)coordinates.size())
            mustRefuel = tree.query(firstHigh, (int)coordinates.size() - 1);

        answer[vertices[i]] += mustRefuel * (n - i - 1LL);

        if (firstHigh < (int)coordinates.size())
            tree.clearRange(firstHigh, (int)coordinates.size() - 1);

        if (mustRefuel)
            tree.add(position(-prefix[i]), mustRefuel);

        tree.add(position(-prefix[i + 1]), 1);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    int64 K;
    cin >> N >> K;

    vector<vector<Edge>> graph(N);
    vector<tuple<int, int, int>> edges;
    edges.reserve(N - 1);

    for (int i = 0; i < N - 1; ++i) {
        int u, v, len;
        cin >> u >> v >> len;
        graph[u].push_back({v, len});
        graph[v].push_back({u, len});
        edges.emplace_back(u, v, len);
    }

    vector<int64> answer(N, 0);

    int maximumDegree = 0;
    for (const auto& adjacency : graph)
        maximumDegree = max(maximumDegree, (int)adjacency.size());

    if (maximumDegree <= 2) {
        int start = 0;
        while ((int)graph[start].size() != 1) ++start;

        vector<int> vertices;
        vector<int> lengths;
        int previous = -1;
        int current = start;

        while (true) {
            vertices.push_back(current);
            int next = -1;
            int nextLength = 0;
            for (const Edge& edge : graph[current]) {
                if (edge.to != previous) {
                    next = edge.to;
                    nextLength = edge.len;
                    break;
                }
            }
            if (next == -1) break;
            lengths.push_back(nextLength);
            previous = current;
            current = next;
        }

        solvePath(vertices, lengths, K, answer);

        reverse(vertices.begin(), vertices.end());
        reverse(lengths.begin(), lengths.end());
        solvePath(vertices, lengths, K, answer);

        for (int i = 0; i < N; ++i)
            cout << answer[i] << '\n';
        return 0;
    }

    vector<int> parent(N, -1), parentLength(N, 0), order;
    vector<int> subtree(N, 1);
    order.reserve(N);

    parent[0] = 0;
    vector<int> stack = {0};
    while (!stack.empty()) {
        int v = stack.back();
        stack.pop_back();
        order.push_back(v);
        for (const Edge& edge : graph[v]) {
            if (edge.to == parent[v]) continue;
            parent[edge.to] = v;
            parentLength[edge.to] = edge.len;
            stack.push_back(edge.to);
        }
    }

    for (int i = N - 1; i > 0; --i) {
        int v = order[i];
        subtree[parent[v]] += subtree[v];
    }

    if (N <= 2500) {
        struct State {
            int vertex;
            int previous;
            int64 used;
        };

        for (int source = 0; source < N; ++source) {
            vector<State> dfs;
            dfs.push_back({source, -1, 0});

            while (!dfs.empty()) {
                State state = dfs.back();
                dfs.pop_back();

                for (const Edge& edge : graph[state.vertex]) {
                    if (edge.to == state.previous) continue;

                    int64 nextUsed;
                    if (state.used + edge.len > K) {
                        int64 destinationCount;
                        if (parent[edge.to] == state.vertex)
                            destinationCount = subtree[edge.to];
                        else
                            destinationCount = N - subtree[state.vertex];

                        answer[state.vertex] += destinationCount;
                        nextUsed = edge.len;
                    } else {
                        nextUsed = state.used + edge.len;
                    }

                    dfs.push_back({edge.to, state.vertex, nextUsed});
                }
            }
        }

        for (int i = 0; i < N; ++i)
            cout << answer[i] << '\n';
        return 0;
    }

    int states = (K <= INT_MAX ? (int)K + 1 : INT_MAX);
    const int64 MAX_CELLS = 15000000LL;

    if (states != INT_MAX && 2LL * N * states <= MAX_CELLS) {
        int S = states;
        vector<int> down((size_t)N * S, 0);
        vector<int> full((size_t)N * S, 0);

        auto at = [S](vector<int>& array, int vertex, int fuel) -> int& {
            return array[(size_t)vertex * S + fuel];
        };

        auto get = [S](const vector<int>& array, int vertex, int fuel) -> int {
            return array[(size_t)vertex * S + fuel];
        };

        for (int idx = N - 1; idx >= 0; --idx) {
            int v = order[idx];
            at(down, v, 0) += 1;

            for (const Edge& edge : graph[v]) {
                int child = edge.to;
                if (parent[child] != v) continue;

                for (int used = 0; used < S; ++used) {
                    int count = get(down, child, used);
                    if (!count) continue;
                    int resulting = (used + edge.len <= K)
                                  ? used + edge.len
                                  : edge.len;
                    at(down, v, resulting) += count;
                }
            }
        }

        for (int used = 0; used < S; ++used)
            at(full, 0, used) = get(down, 0, used);

        for (int idx = 0; idx < N; ++idx) {
            int v = order[idx];

            for (const Edge& edge : graph[v]) {
                int child = edge.to;
                if (parent[child] != v) continue;

                for (int used = 0; used < S; ++used)
                    at(full, child, used) = get(down, child, used);

                for (int used = 0; used < S; ++used) {
                    int childMessage = 0;
                    for (int childState = 0; childState < S; ++childState) {
                        int resulting = (childState + edge.len <= K)
                                      ? childState + edge.len
                                      : edge.len;
                        if (resulting == used)
                            childMessage += get(down, child, childState);
                    }

                    int outsideCount = get(full, v, used) - childMessage;
                    if (!outsideCount) continue;

                    int resulting = (used + edge.len <= K)
                                  ? used + edge.len
                                  : edge.len;
                    at(full, child, resulting) += outsideCount;
                }
            }
        }

        for (int v = 0; v < N; ++v) {
            for (const Edge& edge : graph[v]) {
                int64 destinationCount;
                bool neighborIsChild = (parent[edge.to] == v);

                if (neighborIsChild)
                    destinationCount = subtree[edge.to];
                else
                    destinationCount = N - subtree[v];

                int threshold = (int)(K - edge.len);
                int64 qualifying = 0;

                for (int used = threshold + 1; used < S; ++used) {
                    int excluded = 0;

                    if (neighborIsChild) {
                        for (int childState = 0; childState < S; ++childState) {
                            int resulting =
                                (childState + edge.len <= K)
                                ? childState + edge.len
                                : edge.len;
                            if (resulting == used)
                                excluded += get(down, edge.to, childState);
                        }
                    } else {
                        excluded = get(full, v, used) - get(down, v, used);
                    }

                    qualifying += get(full, v, used) - excluded;
                }

                answer[v] += qualifying * destinationCount;
            }
        }

        for (int i = 0; i < N; ++i)
            cout << answer[i] << '\n';
        return 0;
    }

    for (int i = 0; i < N; ++i)
        cout << 0 << '\n';

    return 0;
}
