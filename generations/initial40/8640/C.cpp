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

class AssignSumSegmentTree {
private:
    int n;
    vector<int64> sum;
    vector<int64> lazy;
    vector<unsigned char> hasLazy;

    void apply(int node, int left, int right, int64 value) {
        sum[node] = value * (right - left + 1LL);
        lazy[node] = value;
        hasLazy[node] = 1;
    }

    void push(int node, int left, int right) {
        if (!hasLazy[node] || left == right) return;
        int mid = (left + right) / 2;
        apply(node * 2, left, mid, lazy[node]);
        apply(node * 2 + 1, mid + 1, right, lazy[node]);
        hasLazy[node] = 0;
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

        sum[node] = sum[node * 2] + sum[node * 2 + 1];
    }

    int64 querySum(int node, int left, int right,
                   int queryLeft, int queryRight) {
        if (queryLeft <= left && right <= queryRight) {
            return sum[node];
        }

        push(node, left, right);
        int mid = (left + right) / 2;
        int64 result = 0;

        if (queryLeft <= mid) {
            result += querySum(node * 2, left, mid,
                               queryLeft, queryRight);
        }
        if (queryRight > mid) {
            result += querySum(node * 2 + 1, mid + 1, right,
                               queryLeft, queryRight);
        }

        return result;
    }

public:
    explicit AssignSumSegmentTree(int size)
        : n(size),
          sum(4 * size + 4, 0),
          lazy(4 * size + 4, 0),
          hasLazy(4 * size + 4, 0) {}

    void assignRange(int left, int right, int64 value) {
        assignRange(1, 1, n, left, right, value);
    }

    int64 querySum(int left, int right) {
        return querySum(1, 1, n, left, right);
    }
};

class RangeMinimumTree {
private:
    int size;
    const int64 INF = numeric_limits<int64>::max() / 4;
    vector<int64> tree;

public:
    explicit RangeMinimumTree(const vector<int64>& values) {
        int n = static_cast<int>(values.size()) - 1;
        size = 1;
        while (size < n) size *= 2;

        tree.assign(2 * size, INF);
        for (int i = 1; i <= n; ++i) {
            tree[size + i - 1] = values[i];
        }
        for (int i = size - 1; i >= 1; --i) {
            tree[i] = min(tree[i * 2], tree[i * 2 + 1]);
        }
    }

    int64 query(int left, int right) const {
        int l = size + left - 1;
        int r = size + right - 1;
        int64 result = INF;

        while (l <= r) {
            if (l & 1) result = min(result, tree[l++]);
            if (!(r & 1)) result = min(result, tree[r--]);
            l /= 2;
            r /= 2;
        }

        return result;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    int64 D;
    cin >> N >> D;

    vector<int64> C(N + 1);
    vector<int64> quotient(N + 1);
    vector<int64> residue(N + 1);
    vector<int64> h(N + 1);
    vector<int64> prefixH(N + 1, 0);
    vector<int> descents(N + 1, 0);

    for (int i = 1; i <= N; ++i) {
        cin >> C[i];
        quotient[i] = C[i] / D;
        residue[i] = C[i] % D;
    }

    for (int i = 1; i <= N; ++i) {
        descents[i] = (i == 1 ? 0 : descents[i - 1]);
        if (i >= 2 && residue[i] < residue[i - 1]) {
            ++descents[i];
        }

        h[i] = quotient[i] - descents[i];
        prefixH[i] = prefixH[i - 1] + h[i];
    }

    int Q;
    cin >> Q;

    vector<vector<Query>> queriesByRight(N + 1);
    vector<int64> answer(Q, -1);

    for (int id = 0; id < Q; ++id) {
        int L, R;
        cin >> L >> R;
        queriesByRight[R].push_back({L, id});
    }

    RangeMinimumTree minimumTree(h);
    AssignSumSegmentTree suffixMinimumTree(N);

    // Each pair is {group value, first index of the group}.
    vector<pair<int64, int>> groups;
    groups.reserve(N);

    for (int right = 1; right <= N; ++right) {
        int firstChanged = right;

        while (!groups.empty() && groups.back().first > h[right]) {
            firstChanged = groups.back().second;
            groups.pop_back();
        }

        suffixMinimumTree.assignRange(firstChanged, right, h[right]);

        if (groups.empty() || groups.back().first < h[right]) {
            groups.push_back({h[right], firstChanged});
        }
        // If equal, the existing final group simply extends to 'right'.

        for (const Query& query : queriesByRight[right]) {
            int left = query.l;

            int64 minimumH = minimumTree.query(left, right);
            if (minimumH + descents[left] < 0) {
                answer[query.id] = -1;
                continue;
            }

            int64 sumH = prefixH[right] - prefixH[left - 1];
            int64 sumSuffixMinimums =
                suffixMinimumTree.querySum(left, right);

            answer[query.id] = sumH - sumSuffixMinimums;
        }
    }

    for (int i = 0; i < Q; ++i) {
        cout << answer[i] << '\n';
    }

    return 0;
}