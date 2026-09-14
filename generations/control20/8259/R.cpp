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

struct Interval {
    int l, r;

    bool operator<(const Interval& other) const {
        if (r != other.r) return r < other.r;
        return l < other.l;
    }

    bool operator==(const Interval& other) const {
        return l == other.l && r == other.r;
    }
};

static string readBits(int length) {
    string result, token;
    while ((int)result.size() < length) {
        cin >> token;
        result += token;
    }
    return result;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int H, W, Q;
    cin >> H >> W >> Q;

    const int N = H * W;
    DSU dsu(N);

    auto id = [W](int row, int col) {
        return row * W + col;
    };

    for (int i = 0; i < H; ++i) {
        string s = readBits(W - 1);
        for (int j = 0; j + 1 < W; ++j) {
            if (s[j] == '1') {
                dsu.unite(id(i, j), id(i, j + 1));
            }
        }
    }

    for (int i = 0; i + 1 < H; ++i) {
        string s = readBits(W);
        for (int j = 0; j < W; ++j) {
            if (s[j] == '1') {
                dsu.unite(id(i, j), id(i + 1, j));
            }
        }
    }

    vector<int> cost(H + 1);
    bool allOne = true;
    for (int i = 1; i <= H; ++i) {
        cin >> cost[i];
        allOne &= (cost[i] == 1);
    }

    vector<int> top(N, H + 1), bottom(N, 0);
    for (int i = 0; i < H; ++i) {
        for (int j = 0; j < W; ++j) {
            int root = dsu.find(id(i, j));
            top[root] = min(top[root], i + 1);
            bottom[root] = max(bottom[root], i + 1);
        }
    }

    /*
      Every original connected component intersects a consecutive interval
      of rows [top, bottom]. Repairing row x connects all components whose
      intervals contain x.

      reach[x] is the farthest row reachable from repaired row x without
      another repair. It is nondecreasing.
    */
    vector<int> startMaximum(H + 2, 0);
    for (int v = 0; v < N; ++v) {
        if (dsu.find(v) == v && bottom[v] != 0) {
            startMaximum[top[v]] = max(startMaximum[top[v]], bottom[v]);
        }
    }

    vector<int> reach(H + 1);
    int farthest = 0;
    for (int row = 1; row <= H; ++row) {
        farthest = max(farthest, startMaximum[row]);
        reach[row] = farthest;
    }

    int LOG = 1;
    while ((1 << LOG) <= H) ++LOG;

    vector<vector<int>> jump(LOG, vector<int>(H + 1));
    vector<vector<long long>> jumpCost(LOG, vector<long long>(H + 1));

    for (int row = 1; row <= H; ++row) {
        jump[0][row] = reach[row];
        jumpCost[0][row] = cost[row];
    }

    for (int k = 1; k < LOG; ++k) {
        for (int row = 1; row <= H; ++row) {
            int middle = jump[k - 1][row];
            jump[k][row] = jump[k - 1][middle];
            jumpCost[k][row] =
                jumpCost[k - 1][row] + jumpCost[k - 1][middle];
        }
    }

    auto makeIntervals = [&](vector<int>& roots) {
        sort(roots.begin(), roots.end());
        roots.erase(unique(roots.begin(), roots.end()), roots.end());

        vector<Interval> intervals;
        intervals.reserve(roots.size());
        for (int root : roots) {
            intervals.push_back({top[root], bottom[root]});
        }

        sort(intervals.begin(), intervals.end());
        intervals.erase(unique(intervals.begin(), intervals.end()),
                        intervals.end());
        return intervals;
    };

    /*
      Exact greedy algorithm when every repair costs one.

      Process terminal intervals by increasing right endpoint. If the next
      interval is already reachable, repair its rightmost reachable row.
      Otherwise, repairing the current frontier is optimal because reach[]
      is nondecreasing. Binary lifting skips many such frontier repairs.
    */
    auto solveUnit = [&](vector<Interval> intervals) -> long long {
        int m = (int)intervals.size();

        int selected = intervals[0].r;
        int frontier = reach[selected];
        long long answer = 1;

        int ptr = 0;
        while (ptr < m && intervals[ptr].l <= selected) ++ptr;

        while (ptr < m) {
            int targetLeft = intervals[ptr].l;

            if (frontier < targetLeft) {
                int current = frontier;

                for (int k = LOG - 1; k >= 0; --k) {
                    int destination = jump[k][current];
                    if (destination > current &&
                        destination < targetLeft) {
                        answer += (1LL << k);
                        current = destination;
                    }
                }

                if (current < targetLeft) {
                    int destination = reach[current];
                    if (destination == current) return -1;
                    ++answer;
                    current = destination;
                }

                frontier = current;
            }

            selected = min(intervals[ptr].r, frontier);
            ++answer;
            frontier = max(frontier, reach[selected]);

            while (ptr < m && intervals[ptr].l <= selected) ++ptr;
        }

        return answer;
    };

    /*
      Exact O(H log H) dynamic programming for a query.

      Let q < p be consecutive repaired rows. They can be connected exactly
      when p <= reach[q]. No terminal interval may lie strictly inside the
      gap (q,p), since it would never be hit by a repaired row.

      gapLimit[q] is therefore the smaller of reach[q] and the minimum right
      endpoint among terminal intervals whose left endpoint is greater than q.
      A state q can transition to every p in [q+1, gapLimit[q]].

      A heap maintains the cheapest currently active predecessor.
    */
    auto solveDP = [&](const vector<Interval>& intervals) -> long long {
        const long long INF = (1LL << 60);

        int minimumRight = H + 1;
        int maximumLeft = 0;

        vector<int> rightAtLeft(H + 2, H + 1);
        for (const Interval& interval : intervals) {
            minimumRight = min(minimumRight, interval.r);
            maximumLeft = max(maximumLeft, interval.l);
            rightAtLeft[interval.l] =
                min(rightAtLeft[interval.l], interval.r);
        }

        vector<int> suffixMinimum(H + 3, H + 1);
        for (int row = H; row >= 1; --row) {
            suffixMinimum[row] =
                min(rightAtLeft[row], suffixMinimum[row + 1]);
        }

        vector<int> gapLimit(H + 1);
        for (int row = 1; row <= H; ++row) {
            gapLimit[row] = min(reach[row], suffixMinimum[row + 1]);
        }

        using State = pair<long long, int>; // (cost, expiry)
        priority_queue<State, vector<State>, greater<State>> heap;

        vector<long long> dp(H + 1, INF);
        long long answer = INF;

        for (int row = 1; row <= H; ++row) {
            while (!heap.empty() && heap.top().second < row) {
                heap.pop();
            }

            long long predecessor = INF;
            if (row <= minimumRight) predecessor = 0;
            if (!heap.empty()) predecessor = min(predecessor, heap.top().first);

            if (predecessor != INF) {
                dp[row] = predecessor + cost[row];

                if (row >= maximumLeft) {
                    answer = min(answer, dp[row]);
                }

                if (gapLimit[row] >= row + 1) {
                    heap.push({dp[row], gapLimit[row]});
                }
            }
        }

        return answer == INF ? -1 : answer;
    };

    /*
      Feasible fallback for very large weighted instances outside the exact
      subtask regimes. It uses the same rightward greedy route while summing
      the actual row costs.
    */
    auto solveWeightedGreedy = [&](vector<Interval> intervals) -> long long {
        int m = (int)intervals.size();

        int selected = intervals[0].r;
        int frontier = reach[selected];
        long long answer = cost[selected];

        int ptr = 0;
        while (ptr < m && intervals[ptr].l <= selected) ++ptr;

        while (ptr < m) {
            int targetLeft = intervals[ptr].l;

            if (frontier < targetLeft) {
                int current = frontier;

                for (int k = LOG - 1; k >= 0; --k) {
                    int destination = jump[k][current];
                    if (destination > current &&
                        destination < targetLeft) {
                        answer += jumpCost[k][current];
                        current = destination;
                    }
                }

                if (current < targetLeft) {
                    int destination = reach[current];
                    if (destination == current) return -1;
                    answer += cost[current];
                    current = destination;
                }

                frontier = current;
            }

            selected = min(intervals[ptr].r, frontier);
            answer += cost[selected];
            frontier = max(frontier, reach[selected]);

            while (ptr < m && intervals[ptr].l <= selected) ++ptr;
        }

        return answer;
    };

    /*
      The O(H log H) method is guaranteed fast for Q <= 5. It is also used
      on larger inputs whenever the total scan size remains moderate.
    */
    const long long DP_WORK_LIMIT = 30000000LL;
    bool useExactDPForWeighted =
        (Q <= 5 || 1LL * Q * H <= DP_WORK_LIMIT);

    for (int query = 0; query < Q; ++query) {
        int T;
        cin >> T;

        vector<int> roots;
        roots.reserve(T);

        for (int i = 0; i < T; ++i) {
            int x, y;
            cin >> x >> y;
            roots.push_back(dsu.find(id(x - 1, y - 1)));
        }

        sort(roots.begin(), roots.end());
        roots.erase(unique(roots.begin(), roots.end()), roots.end());

        if (roots.size() == 1) {
            cout << 0 << '\n';
            continue;
        }

        vector<Interval> intervals = makeIntervals(roots);

        long long answer;
        if (allOne) {
            answer = solveUnit(intervals);
        } else if (useExactDPForWeighted) {
            answer = solveDP(intervals);
        } else {
            answer = solveWeightedGreedy(intervals);
        }

        cout << answer << '\n';
    }

    return 0;
}
