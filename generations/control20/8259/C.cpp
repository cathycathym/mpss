#include <bits/stdc++.h>
using namespace std;

struct DSU {
    vector<int> p, sz;

    explicit DSU(int n) : p(n), sz(n, 1) {
        iota(p.begin(), p.end(), 0);
    }

    int find(int x) {
        while (p[x] != x) {
            p[x] = p[p[x]];
            x = p[x];
        }
        return x;
    }

    void unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return;
        if (sz[a] < sz[b]) swap(a, b);
        p[b] = a;
        sz[a] += sz[b];
    }
};

struct Candidate {
    long long value;
    int limit;

    bool operator>(const Candidate& other) const {
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
    DSU dsu(N);

    auto id = [W](int r, int c) {
        return r * W + c;
    };

    string s;

    for (int r = 0; r < H; ++r) {
        cin >> s;
        for (int c = 0; c + 1 < W; ++c) {
            if (s[c] == '1') {
                dsu.unite(id(r, c), id(r, c + 1));
            }
        }
    }

    for (int r = 0; r + 1 < H; ++r) {
        cin >> s;
        for (int c = 0; c < W; ++c) {
            if (s[c] == '1') {
                dsu.unite(id(r, c), id(r + 1, c));
            }
        }
    }

    vector<int> cost(H);
    for (int& x : cost) cin >> x;

    vector<int> componentIndex(N, -1);
    vector<int> vertexComponent(N);
    vector<int> top, bottom;

    int componentCount = 0;
    for (int r = 0; r < H; ++r) {
        for (int c = 0; c < W; ++c) {
            int v = id(r, c);
            int root = dsu.find(v);

            if (componentIndex[root] == -1) {
                componentIndex[root] = componentCount++;
                top.push_back(r);
                bottom.push_back(r);
            }

            int component = componentIndex[root];
            vertexComponent[v] = component;
            top[component] = min(top[component], r);
            bottom[component] = max(bottom[component], r);
        }
    }

    /*
       rightReach[r] is the maximum bottom endpoint among all original
       components touching row r.
    */
    vector<vector<int>> starts(H);
    for (int component = 0; component < componentCount; ++component) {
        starts[top[component]].push_back(bottom[component]);
    }

    vector<int> rightReach(H);
    priority_queue<int> activeBottoms;

    for (int r = 0; r < H; ++r) {
        for (int b : starts[r]) activeBottoms.push(b);

        while (!activeBottoms.empty() && activeBottoms.top() < r) {
            activeBottoms.pop();
        }

        rightReach[r] = activeBottoms.empty() ? r : activeBottoms.top();
    }

    const int INF_ROW = H + 5;
    const long long INF = (1LL << 60);

    vector<int> minimumRightAtLeft(H);
    vector<int> deadline(H);
    vector<long long> dp(H);

    while (Q--) {
        int T;
        cin >> T;

        vector<int> terminalComponents;
        terminalComponents.reserve(T);

        for (int i = 0; i < T; ++i) {
            int x, y;
            cin >> x >> y;
            --x;
            --y;
            terminalComponents.push_back(vertexComponent[id(x, y)]);
        }

        sort(terminalComponents.begin(), terminalComponents.end());
        terminalComponents.erase(
            unique(terminalComponents.begin(), terminalComponents.end()),
            terminalComponents.end()
        );

        if (terminalComponents.size() == 1) {
            cout << 0 << '\n';
            continue;
        }

        fill(minimumRightAtLeft.begin(), minimumRightAtLeft.end(), INF_ROW);

        int minimumRight = H - 1;
        int maximumLeft = 0;

        for (int component : terminalComponents) {
            int l = top[component];
            int r = bottom[component];

            minimumRightAtLeft[l] = min(minimumRightAtLeft[l], r);
            minimumRight = min(minimumRight, r);
            maximumLeft = max(maximumLeft, l);
        }

        /*
          deadline[p] = min r_t over terminal intervals with l_t > p.
          If no such interval exists, there is no terminal-imposed limit.
        */
        int suffixMinimum = INF_ROW;
        for (int p = H - 1; p >= 0; --p) {
            deadline[p] = suffixMinimum;
            suffixMinimum = min(suffixMinimum, minimumRightAtLeft[p]);
        }

        priority_queue<
            Candidate,
            vector<Candidate>,
            greater<Candidate>
        > heap;

        long long answer = INF;

        for (int p = 0; p < H; ++p) {
            while (!heap.empty() && heap.top().limit < p) {
                heap.pop();
            }

            long long best = INF;

            /*
              The first repaired row must not be to the right of the
              smallest terminal right endpoint.
            */
            if (p <= minimumRight) {
                best = cost[p];
            }

            if (!heap.empty()) {
                best = min(best, heap.top().value + cost[p]);
            }

            dp[p] = best;

            if (best == INF) continue;

            /*
              Once p is at or to the right of every terminal's left
              endpoint, all terminal intervals have been stabbed.
            */
            if (p >= maximumLeft) {
                answer = min(answer, best);
            }

            int transitionLimit = min(rightReach[p], deadline[p]);
            if (transitionLimit > p) {
                heap.push({best, transitionLimit});
            }
        }

        if (answer == INF) cout << -1 << '\n';
        else cout << answer << '\n';
    }

    return 0;
}
