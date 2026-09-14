#include <bits/stdc++.h>
using namespace std;

struct MaxPairSegTree {
    int size;
    vector<pair<int, int>> tree;

    MaxPairSegTree(int n = 0) {
        init(n);
    }

    void init(int n) {
        size = 1;
        while (size < n) size <<= 1;
        tree.assign(2 * size, {-1, -1});
    }

    void setPoint(int pos, pair<int, int> value) {
        int p = pos + size - 1;
        tree[p] = value;
        for (p >>= 1; p; p >>= 1) {
            tree[p] = max(tree[p << 1], tree[p << 1 | 1]);
        }
    }

    pair<int, int> query(int l, int r) const {
        if (l > r) return {-1, -1};

        l += size - 1;
        r += size - 1;
        pair<int, int> result = {-1, -1};

        while (l <= r) {
            if (l & 1) result = max(result, tree[l++]);
            if (!(r & 1)) result = max(result, tree[r--]);
            l >>= 1;
            r >>= 1;
        }
        return result;
    }
};

struct MaxSegTree {
    int size;
    vector<int> tree;

    MaxSegTree(int n = 0) {
        init(n);
    }

    void init(int n) {
        size = 1;
        while (size < n) size <<= 1;
        tree.assign(2 * size, -1);
    }

    void setPoint(int pos, int value) {
        int p = pos + size - 1;
        tree[p] = value;
        for (p >>= 1; p; p >>= 1) {
            tree[p] = max(tree[p << 1], tree[p << 1 | 1]);
        }
    }

    int query(int l, int r) const {
        if (l > r) return -1;

        l += size - 1;
        r += size - 1;
        int result = -1;

        while (l <= r) {
            if (l & 1) result = max(result, tree[l++]);
            if (!(r & 1)) result = max(result, tree[r--]);
            l >>= 1;
            r >>= 1;
        }
        return result;
    }
};

struct Query {
    int l, r, id;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    cin >> N;

    vector<int> A(N + 1), B(N + 1);
    for (int i = 1; i <= N; ++i) cin >> A[i];
    for (int i = 1; i <= N; ++i) cin >> B[i];

    const int MAX_VALUE = 2 * N;
    vector<int> prv(N + 1, 0);
    vector<int> nxt(N + 1, N + 1);

    /*
     * Left-to-right sweep.
     *
     * The segment tree contains students that have not yet found an
     * overlapping student to their right. It is indexed by B.
     *
     * Student i overlaps current student j iff:
     *     B_i < A_j and A_i > B_j.
     */
    MaxPairSegTree candidates(MAX_VALUE);

    for (int j = 1; j <= N; ++j) {
        while (true) {
            auto best = candidates.query(1, A[j] - 1);
            if (best.first <= B[j]) break;

            int i = best.second;
            nxt[i] = j;
            candidates.setPoint(B[i], {-1, -1});
        }

        candidates.setPoint(B[j], {A[j], j});
    }

    /*
     * Right-to-left sweep, symmetrically computing the nearest
     * overlapping student to the left.
     */
    candidates.init(MAX_VALUE);

    for (int j = N; j >= 1; --j) {
        while (true) {
            auto best = candidates.query(1, A[j] - 1);
            if (best.first <= B[j]) break;

            int i = best.second;
            prv[i] = j;
            candidates.setPoint(B[i], {-1, -1});
        }

        candidates.setPoint(B[j], {A[j], j});
    }

    int Q;
    cin >> Q;

    vector<Query> queries(Q);
    for (int i = 0; i < Q; ++i) {
        cin >> queries[i].l >> queries[i].r;
        queries[i].id = i;
    }

    /*
     * For a fixed L, activate all indices i with prv[i] < L.
     * At position i, store nxt[i].
     *
     * A query [L,R] is invalid iff the maximum active nxt[i] over
     * i in [L,R] is greater than R.
     */
    vector<int> indices(N);
    iota(indices.begin(), indices.end(), 1);
    sort(indices.begin(), indices.end(), [&](int x, int y) {
        return prv[x] < prv[y];
    });

    vector<int> queryOrder(Q);
    iota(queryOrder.begin(), queryOrder.end(), 0);
    sort(queryOrder.begin(), queryOrder.end(), [&](int x, int y) {
        return queries[x].l < queries[y].l;
    });

    MaxSegTree active(N);
    vector<bool> answer(Q, false);

    int ptr = 0;
    for (int qi : queryOrder) {
        int L = queries[qi].l;
        int R = queries[qi].r;

        while (ptr < N && prv[indices[ptr]] < L) {
            int i = indices[ptr++];
            active.setPoint(i, nxt[i]);
        }

        int farthestNext = active.query(L, R);
        answer[queries[qi].id] = (farthestNext <= R);
    }

    for (int i = 0; i < Q; ++i) {
        cout << (answer[i] ? "Yes\n" : "No\n");
    }

    return 0;
}
