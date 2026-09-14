#include <bits/stdc++.h>
using namespace std;

class OverlapTree {
    int size;
    vector<int> mx;
    vector<int> owner;

    void setValue(int node, int left, int right,
                  int position, int value, int index) {
        if (left == right) {
            mx[node] = value;
            owner[left] = index;
            return;
        }

        int middle = (left + right) / 2;
        if (position <= middle) {
            setValue(node * 2, left, middle, position, value, index);
        } else {
            setValue(node * 2 + 1, middle + 1, right,
                     position, value, index);
        }
        mx[node] = max(mx[node * 2], mx[node * 2 + 1]);
    }

    int findValue(int node, int left, int right,
                  int queryLeft, int queryRight, int threshold) const {
        if (right < queryLeft || queryRight < left || mx[node] <= threshold) {
            return -1;
        }
        if (left == right) {
            return left;
        }

        int middle = (left + right) / 2;
        int result = findValue(node * 2, left, middle,
                               queryLeft, queryRight, threshold);
        if (result != -1) {
            return result;
        }
        return findValue(node * 2 + 1, middle + 1, right,
                         queryLeft, queryRight, threshold);
    }

public:
    explicit OverlapTree(int n)
        : size(n), mx(4 * n + 4, -1), owner(n + 1, -1) {}

    void insert(int position, int value, int index) {
        setValue(1, 1, size, position, value, index);
    }

    void erase(int position) {
        setValue(1, 1, size, position, -1, -1);
    }

    int findOverlap(int maximumB, int minimumA) const {
        if (maximumB < 1) {
            return -1;
        }
        int position = findValue(1, 1, size, 1, maximumB, minimumA);
        return position == -1 ? -1 : owner[position];
    }
};

class MaximumTree {
    int size;
    vector<int> tree;

public:
    explicit MaximumTree(int n) {
        size = 1;
        while (size < n) {
            size *= 2;
        }
        tree.assign(2 * size, 0);
    }

    void setValue(int position, int value) {
        int node = position - 1 + size;
        tree[node] = value;
        for (node /= 2; node >= 1; node /= 2) {
            tree[node] = max(tree[node * 2], tree[node * 2 + 1]);
        }
    }

    int rangeMaximum(int left, int right) const {
        int result = 0;
        left = left - 1 + size;
        right = right - 1 + size;

        while (left <= right) {
            if (left % 2 == 1) {
                result = max(result, tree[left++]);
            }
            if (right % 2 == 0) {
                result = max(result, tree[right--]);
            }
            left /= 2;
            right /= 2;
        }
        return result;
    }
};

struct Query {
    int right;
    int index;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    cin >> N;

    vector<int> A(N + 1), B(N + 1);
    for (int i = 1; i <= N; ++i) {
        cin >> A[i];
    }
    for (int i = 1; i <= N; ++i) {
        cin >> B[i];
    }

    const int endpointLimit = 2 * N;
    vector<int> nearestLeft(N + 1, 0);
    vector<int> nearestRight(N + 1, N + 1);

    {
        OverlapTree tree(endpointLimit);

        for (int k = 1; k <= N; ++k) {
            while (true) {
                int i = tree.findOverlap(A[k] - 1, B[k]);
                if (i == -1) {
                    break;
                }
                nearestRight[i] = k;
                tree.erase(B[i]);
            }
            tree.insert(B[k], A[k], k);
        }
    }

    {
        OverlapTree tree(endpointLimit);

        for (int k = N; k >= 1; --k) {
            while (true) {
                int i = tree.findOverlap(A[k] - 1, B[k]);
                if (i == -1) {
                    break;
                }
                nearestLeft[i] = k;
                tree.erase(B[i]);
            }
            tree.insert(B[k], A[k], k);
        }
    }

    int Q;
    cin >> Q;

    vector<vector<Query>> queriesByLeft(N + 1);
    for (int j = 0; j < Q; ++j) {
        int L, R;
        cin >> L >> R;
        queriesByLeft[L].push_back({R, j});
    }

    vector<int> firstActivation(N + 2, -1);
    vector<int> nextActivation(N + 1, -1);

    for (int k = 1; k <= N; ++k) {
        int activation = nearestLeft[k] + 1;
        nextActivation[k] = firstActivation[activation];
        firstActivation[activation] = k;
    }

    vector<bool> answer(Q, false);
    MaximumTree queryTree(N);

    for (int L = 1; L <= N; ++L) {
        for (int k = firstActivation[L]; k != -1; k = nextActivation[k]) {
            queryTree.setValue(k, nearestRight[k]);
        }

        for (const Query &query : queriesByLeft[L]) {
            int furthestRight = queryTree.rangeMaximum(L, query.right);
            answer[query.index] = (furthestRight <= query.right);
        }
    }

    for (int j = 0; j < Q; ++j) {
        cout << (answer[j] ? "Yes" : "No") << '\n';
    }

    return 0;
}
