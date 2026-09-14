#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>

using namespace std;

class MaxSegmentTree {
private:
    int n;
    vector<int> tree;

    void update(int node, int left, int right, int position, int value) {
        if (left == right) {
            tree[node] = value;
            return;
        }

        int middle = (left + right) / 2;
        if (position <= middle) {
            update(node * 2, left, middle, position, value);
        } else {
            update(node * 2 + 1, middle + 1, right, position, value);
        }

        tree[node] = max(tree[node * 2], tree[node * 2 + 1]);
    }

    int rangeMaximum(int node, int left, int right,
                     int queryLeft, int queryRight) const {
        if (queryRight < left || right < queryLeft) {
            return 0;
        }
        if (queryLeft <= left && right <= queryRight) {
            return tree[node];
        }

        int middle = (left + right) / 2;
        return max(
            rangeMaximum(node * 2, left, middle, queryLeft, queryRight),
            rangeMaximum(node * 2 + 1, middle + 1, right,
                         queryLeft, queryRight)
        );
    }

    int findRightmost(int node, int left, int right,
                      int queryLeft, int queryRight, int threshold) const {
        if (queryRight < left || right < queryLeft ||
            tree[node] <= threshold) {
            return 0;
        }

        if (left == right) {
            return left;
        }

        int middle = (left + right) / 2;

        int result = findRightmost(
            node * 2 + 1, middle + 1, right,
            queryLeft, queryRight, threshold
        );

        if (result != 0) {
            return result;
        }

        return findRightmost(
            node * 2, left, middle,
            queryLeft, queryRight, threshold
        );
    }

    int findLeftmost(int node, int left, int right,
                     int queryLeft, int queryRight, int threshold) const {
        if (queryRight < left || right < queryLeft ||
            tree[node] <= threshold) {
            return n + 1;
        }

        if (left == right) {
            return left;
        }

        int middle = (left + right) / 2;

        int result = findLeftmost(
            node * 2, left, middle,
            queryLeft, queryRight, threshold
        );

        if (result != n + 1) {
            return result;
        }

        return findLeftmost(
            node * 2 + 1, middle + 1, right,
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
        if (left > right) {
            return 0;
        }
        return rangeMaximum(1, 1, n, left, right);
    }

    int findRightmost(int left, int right, int threshold) const {
        if (left > right) {
            return 0;
        }
        return findRightmost(1, 1, n, left, right, threshold);
    }

    int findLeftmost(int left, int right, int threshold) const {
        if (left > right) {
            return n + 1;
        }
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

    vector<int> A(N + 1);
    vector<int> B(N + 1);

    for (int i = 1; i <= N; ++i) {
        cin >> A[i];
    }
    for (int i = 1; i <= N; ++i) {
        cin >> B[i];
    }

    vector<int> byB(N), byA(N);
    iota(byB.begin(), byB.end(), 1);
    iota(byA.begin(), byA.end(), 1);

    sort(byB.begin(), byB.end(), [&](int x, int y) {
        return B[x] < B[y];
    });
    sort(byA.begin(), byA.end(), [&](int x, int y) {
        return A[x] < A[y];
    });

    /*
       Intervals i and j intersect exactly when
           B_j < A_i and A_j > B_i.
       While processing i in increasing A_i, activate every j satisfying
       B_j < A_i. The segment tree then locates the nearest activated
       position whose A_j > B_i.
    */
    MaxSegmentTree intersectionTree(N);
    vector<int> previousIntersection(N + 1, 0);
    vector<int> nextIntersection(N + 1, N + 1);

    int pointer = 0;
    for (int i : byA) {
        while (pointer < N && B[byB[pointer]] < A[i]) {
            int j = byB[pointer++];
            intersectionTree.update(j, A[j]);
        }

        previousIntersection[i] =
            intersectionTree.findRightmost(1, i - 1, B[i]);
        nextIntersection[i] =
            intersectionTree.findLeftmost(i + 1, N, B[i]);
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
    sort(students.begin(), students.end(), [&](int x, int y) {
        return previousIntersection[x] < previousIntersection[y];
    });

    /*
       For query [L,R], student i is isolated inside the query iff
           previousIntersection[i] < L
       and
           nextIntersection[i] > R.

       Activate students satisfying the first condition, then take the
       maximum nextIntersection over positions [L,R].
    */
    MaxSegmentTree queryTree(N);
    vector<bool> answer(Q, false);

    pointer = 0;
    for (const Query& query : queries) {
        while (pointer < N &&
               previousIntersection[students[pointer]] < query.left) {
            int student = students[pointer++];
            queryTree.update(student, nextIntersection[student]);
        }

        int farthestNext =
            queryTree.rangeMaximum(query.left, query.right);

        answer[query.id] = (farthestNext <= query.right);
    }

    for (bool possible : answer) {
        cout << (possible ? "Yes" : "No") << '\n';
    }

    return 0;
}
