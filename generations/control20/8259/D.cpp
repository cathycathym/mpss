#include <bits/stdc++.h>
using namespace std;

struct DSU {
    vector<int> parent, size;

    explicit DSU(int n) : parent(n), size(n, 1) {
        iota(parent.begin(), parent.end(), 0);
    }

    int find(int x) {
        while (parent[x] != x) {
            parent[x] = parent[parent[x]];
            x = parent[x];
        }
        return x;
    }

    void unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return;
        if (size[a] < size[b]) swap(a, b);
        parent[b] = a;
        size[a] += size[b];
    }
};

struct State {
    long long value;
    int limit;

    bool operator>(const State& other) const {
        if (value != other.value) return value > other.value;
        return limit > other.limit;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int H, W, Q;
    cin >> H >> W >> Q;

    const int N = H * W;
    auto vertex = [W](int row, int column) {
        return row * W + column;
    };

    DSU dsu(N);
    string s;

    for (int row = 0; row < H; ++row) {
        cin >> s;
        for (int column = 0; column + 1 < W; ++column) {
            if (s[column] == '1') {
                dsu.unite(vertex(row, column),
                          vertex(row, column + 1));
            }
        }
    }

    for (int row = 0; row + 1 < H; ++row) {
        cin >> s;
        for (int column = 0; column < W; ++column) {
            if (s[column] == '1') {
                dsu.unite(vertex(row, column),
                          vertex(row + 1, column));
            }
        }
    }

    vector<int> cost(H);
    for (int& x : cost) cin >> x;

    vector<int> rootIndex(N, -1);
    vector<int> cellComponent(N);
    vector<int> top, bottom;

    int componentCount = 0;

    for (int row = 0; row < H; ++row) {
        for (int column = 0; column < W; ++column) {
            int v = vertex(row, column);
            int root = dsu.find(v);

            if (rootIndex[root] == -1) {
                rootIndex[root] = componentCount++;
                top.push_back(row);
                bottom.push_back(row);
            }

            int component = rootIndex[root];
            cellComponent[v] = component;
            top[component] = min(top[component], row);
            bottom[component] = max(bottom[component], row);
        }
    }

    vector<vector<int>> componentStarts(H);
    for (int component = 0; component < componentCount; ++component) {
        componentStarts[top[component]].push_back(bottom[component]);
    }

    vector<int> rightReach(H);
    priority_queue<int> activeComponents;

    for (int row = 0; row < H; ++row) {
        for (int endpoint : componentStarts[row]) {
            activeComponents.push(endpoint);
        }

        while (!activeComponents.empty() &&
               activeComponents.top() < row) {
            activeComponents.pop();
        }

        rightReach[row] = activeComponents.empty()
                        ? row
                        : activeComponents.top();
    }

    const long long INF = (1LL << 60);
    vector<long long> dp(H);

    while (Q--) {
        int T;
        cin >> T;

        vector<int> terminals;
        terminals.reserve(T);

        for (int i = 0; i < T; ++i) {
            int x, y;
            cin >> x >> y;
            --x;
            --y;
            terminals.push_back(cellComponent[vertex(x, y)]);
        }

        sort(terminals.begin(), terminals.end());
        terminals.erase(unique(terminals.begin(), terminals.end()),
                        terminals.end());

        if (terminals.size() == 1) {
            cout << 0 << '\n';
            continue;
        }

        vector<pair<int, int>> intervals;
        intervals.reserve(terminals.size());

        int minimumLeft = H;
        int maximumLeft = -1;
        int minimumRight = H;
        int maximumRight = -1;

        for (int component : terminals) {
            int l = top[component];
            int r = bottom[component];

            intervals.push_back({l, r});
            minimumLeft = min(minimumLeft, l);
            maximumLeft = max(maximumLeft, l);
            minimumRight = min(minimumRight, r);
            maximumRight = max(maximumRight, r);
        }

        sort(intervals.begin(), intervals.end());

        vector<int> suffixMinimumRight(intervals.size() + 1, H);
        for (int i = int(intervals.size()) - 1; i >= 0; --i) {
            suffixMinimumRight[i] =
                min(suffixMinimumRight[i + 1], intervals[i].second);
        }

        priority_queue<State, vector<State>, greater<State>> heap;

        long long answer = INF;
        size_t firstFutureInterval = 0;

        for (int row = minimumLeft; row <= maximumRight; ++row) {
            while (firstFutureInterval < intervals.size() &&
                   intervals[firstFutureInterval].first <= row) {
                ++firstFutureInterval;
            }

            while (!heap.empty() && heap.top().limit < row) {
                heap.pop();
            }

            long long best = INF;

            if (row <= minimumRight) {
                best = cost[row];
            }

            if (!heap.empty()) {
                best = min(best, heap.top().value + cost[row]);
            }

            dp[row] = best;
            if (best == INF) continue;

            if (row >= maximumLeft) {
                answer = min(answer, best);
            }

            int terminalDeadline =
                suffixMinimumRight[firstFutureInterval];
            int transitionLimit =
                min(rightReach[row], terminalDeadline);

            if (transitionLimit > row) {
                heap.push({best, transitionLimit});
            }
        }

        if (answer == INF) cout << -1 << '\n';
        else cout << answer << '\n';
    }

    return 0;
}
