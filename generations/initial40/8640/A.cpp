#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
using namespace std;

using int64 = long long;

struct Query {
    int l;
    int id;
};

struct Result {
    int64 sum;
    int64 maxDiff;
};

class SegmentTree {
private:
    struct Node {
        int64 sum = 0;
        int64 minH = 0;
        int64 maxDiff = numeric_limits<int64>::lowest() / 4;
        int64 lazy = 0;
        bool hasLazy = false;
    };

    int n;
    vector<Node> tree;
    const vector<int64>& H;

    void build(int node, int left, int right) {
        if (left == right) {
            tree[node].minH = H[left];
            return;
        }

        int mid = (left + right) / 2;
        build(node * 2, left, mid);
        build(node * 2 + 1, mid + 1, right);
        tree[node].minH =
            min(tree[node * 2].minH, tree[node * 2 + 1].minH);
    }

    void apply(int node, int left, int right, int64 value) {
        tree[node].sum = value * (right - left + 1LL);
        tree[node].maxDiff = value - tree[node].minH;
        tree[node].lazy = value;
        tree[node].hasLazy = true;
    }

    void push(int node, int left, int right) {
        if (!tree[node].hasLazy || left == right) return;

        int mid = (left + right) / 2;
        apply(node * 2, left, mid, tree[node].lazy);
        apply(node * 2 + 1, mid + 1, right, tree[node].lazy);
        tree[node].hasLazy = false;
    }

    void assignRange(int node, int left, int right,
                     int queryLeft, int queryRight, int64 value) {
        if (queryLeft <= left && right <= queryRight) {
            apply(node, left, right, value);
            return;
        }

        push(node, left, right);
        int mid = (left + right) / 2;

        if (queryLeft <= mid) {
            assignRange(node * 2, left, mid,
                        queryLeft, queryRight, value);
        }
        if (queryRight > mid) {
            assignRange(node * 2 + 1, mid + 1, right,
                        queryLeft, queryRight, value);
        }

        tree[node].sum =
            tree[node * 2].sum + tree[node * 2 + 1].sum;
        tree[node].maxDiff =
            max(tree[node * 2].maxDiff,
                tree[node * 2 + 1].maxDiff);
    }

    Result queryRange(int node, int left, int right,
                      int queryLeft, int queryRight) {
        if (queryLeft <= left && right <= queryRight) {
            return {tree[node].sum, tree[node].maxDiff};
        }

        push(node, left, right);
        int mid = (left + right) / 2;

        if (queryRight <= mid) {
            return queryRange(node * 2, left, mid,
                              queryLeft, queryRight);
        }
        if (queryLeft > mid) {
            return queryRange(node * 2 + 1, mid + 1, right,
                              queryLeft, queryRight);
        }

        Result a = queryRange(node * 2, left, mid,
                              queryLeft, queryRight);
        Result b = queryRange(node * 2 + 1, mid + 1, right,
                              queryLeft, queryRight);

        return {a.sum + b.sum, max(a.maxDiff, b.maxDiff)};
    }

public:
    SegmentTree(int size, const vector<int64>& values)
        : n(size), tree(4 * size + 5), H(values) {
        build(1, 1, n);
    }

    void assignRange(int left, int right, int64 value) {
        assignRange(1, 1, n, left, right, value);
    }

    Result queryRange(int left, int right) {
        return queryRange(1, 1, n, left, right);
    }
};

int64 ceilDivision(int64 x, int64 d) {
    if (x >= 0) {
        return x / d + (x % d != 0);
    }
    // C++ division truncates toward zero, which equals ceil(x/d) for x < 0.
    return x / d;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    int64 D;
    cin >> N >> D;

    vector<int64> C(N + 1);
    for (int i = 1; i <= N; ++i) {
        cin >> C[i];
    }

    vector<int64> P(N + 1, 0);
    for (int i = 1; i < N; ++i) {
        int64 w = ceilDivision(C[i] - C[i + 1], D);
        P[i + 1] = P[i] + w;
    }

    // H[i] = P[i] + floor(C[i] / D).
    // Feasibility requires suffixMaximum(P)[i] <= H[i].
    vector<int64> H(N + 1);
    vector<int64> prefixP(N + 1, 0);

    for (int i = 1; i <= N; ++i) {
        H[i] = P[i] + C[i] / D;
        prefixP[i] = prefixP[i - 1] + P[i];
    }

    int Q;
    cin >> Q;

    vector<vector<Query>> queriesByRight(N + 1);
    for (int id = 0; id < Q; ++id) {
        int L, R;
        cin >> L >> R;
        queriesByRight[R].push_back({L, id});
    }

    vector<int64> answers(Q, -1);
    SegmentTree segmentTree(N, H);

    // Stack indices have strictly decreasing P-values.
    vector<int> stack;
    stack.reserve(N);

    for (int right = 1; right <= N; ++right) {
        while (!stack.empty() && P[stack.back()] <= P[right]) {
            stack.pop_back();
        }

        int previousGreater = stack.empty() ? 0 : stack.back();

        // For every i after previousGreater, P[right] is the maximum
        // value in P[i..right].
        segmentTree.assignRange(previousGreater + 1, right, P[right]);
        stack.push_back(right);

        for (const Query& query : queriesByRight[right]) {
            Result result = segmentTree.queryRange(query.l, right);

            if (result.maxDiff <= 0) {
                int64 sumP =
                    prefixP[right] - prefixP[query.l - 1];
                answers[query.id] = result.sum - sumP;
            }
        }
    }

    for (int64 answer : answers) {
        cout << answer << '\n';
    }

    return 0;
}