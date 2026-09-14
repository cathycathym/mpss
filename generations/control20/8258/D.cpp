#include <bits/stdc++.h>
using namespace std;

class MaxSegmentTree {
    int n;
    vector<int> tree;

    void update(int node, int left, int right, int position, int value) {
        if (left == right) {
            tree[node] = value;
            return;
        }

        int mid = (left + right) / 2;
        if (position <= mid) {
            update(node * 2, left, mid, position, value);
        } else {
            update(node * 2 + 1, mid + 1, right, position, value);
        }

        tree[node] = max(tree[node * 2], tree[node * 2 + 1]);
    }

    int rangeMaximum(int node, int left, int right,
                     int queryLeft, int queryRight) const {
        if (queryRight < left || right < queryLeft) return 0;
        if (queryLeft <= left && right <= queryRight) return tree[node];

        int mid = (left + right) / 2;
        return max(
            rangeMaximum(node * 2, left, mid, queryLeft, queryRight),
            rangeMaximum(node * 2 + 1, mid + 1, right,
                         queryLeft, queryRight)
        );
    }

    int findRightmost(int node, int left, int right,
                      int queryLeft, int queryRight, int threshold) const {
        if (queryRight < left || right < queryLeft ||
            tree[node] <= threshold) {
            return 0;
        }

        if (left == right) return left;

        int mid = (left + right) / 2;

        int result = findRightmost(
            node * 2 + 1, mid + 1, right,
            queryLeft, queryRight, threshold
        );

        if (result != 0) return result;

        return findRightmost(
            node * 2, left, mid,
            queryLeft, queryRight, threshold
        );
    }

    int findLeftmost(int node, int left, int right,
                     int queryLeft, int queryRight, int threshold) const {
        if (queryRight < left || right < queryLeft ||
            tree[node] <= threshold) {
            return n + 1;
        }

        if (left == right) return left;

        int mid = (left + right) / 2;

        int result = findLeftmost(
            node * 2, left, mid,
            queryLeft, queryRight, threshold
        );

        if (result != n + 1) return result;

        return findLeftmost(
            node * 2 + 1, mid + 1, right,
            queryLeft, queryRight, threshold
        );
    }

public:
    explicit MaxSegmentTree(int size)
        : n(size), tree(4 * size + 4, 0) {}

    void update(int position, int value) {
        update(1, 1, n, position, value);
    }

    int rangeMaximum(int left, int right) const {
        if (left > right) return 0;
        return rangeMaximum(1, 1, n, left, right);
    }

    int findRightmost(int left, int right, int threshold) const {
        if (left > right) return 0;
        return findRightmost(1, 1, n, left, right, threshold);
    }

    int findLeftmost(int left, int right, int threshold) const {
        if (left > right) return n + 1;
        return findLeftmost(1, 1, n, left, right, threshold);
    }
};

struct Query {
    int left;
    int right;
    int id;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    cin >> N;

    vector<int> A(N + 1), B(N + 1);
    for (int i = 1; i <= N; ++i) cin >> A[i];
    for (int i = 1; i <= N; ++i) cin >> B[i];

    vector<int> byA(N), byB(N);
    iota(byA.begin(), byA.end(), 1);
    iota(byB.begin(), byB.end(), 1);

    sort(byA.begin(), byA.end(),
         [&](int x, int y) { return A[x] < A[y]; });
    sort(byB.begin(), byB.end(),
         [&](int x, int y) { return B[x] < B[y]; });

    vector<int> nearestLeft(N + 1, 0);
    vector<int> nearestRight(N + 1, N + 1);

    MaxSegmentTree overlapTree(N);
    int pointer = 0;

    for (int i : byA) {
        while (pointer < N && B[byB[pointer]] < A[i]) {
            int j = byB[pointer++];
            overlapTree.update(j, A[j]);
        }

        nearestLeft[i] =
            overlapTree.findRightmost(1, i - 1, B[i]);
        nearestRight[i] =
            overlapTree.findLeftmost(i + 1, N, B[i]);
    }

    int Q;
    cin >> Q;

    vector<Query> queries(Q);
    for (int i = 0; i < Q; ++i) {
        cin >> queries[i].left >> queries[i].right;
        queries[i].id = i;
    }

    sort(queries.begin(), queries.end(),
         [](const Query& x, const Query& y) {
             return x.left < y.left;
         });

    vector<int> students(N);
    iota(students.begin(), students.end(), 1);
    sort(students.begin(), students.end(),
         [&](int x, int y) {
             return nearestLeft[x] < nearestLeft[y];
         });

    MaxSegmentTree queryTree(N);
    vector<bool> answer(Q);
    pointer = 0;

    for (const Query& query : queries) {
        while (pointer < N &&
               nearestLeft[students[pointer]] < query.left) {
            int student = students[pointer++];
            queryTree.update(student, nearestRight[student]);
        }

        int farthestRight =
            queryTree.rangeMaximum(query.left, query.right);

        answer[query.id] = (farthestRight <= query.right);
    }

    for (bool possible : answer) {
        cout << (possible ? "Yes" : "No") << '\n';
    }

    return 0;
}
