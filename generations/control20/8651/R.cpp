#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

struct Node {
    int degree;
    int id;
};

static bool nodeLess(const Node& a, const Node& b) {
    if (a.degree != b.degree) return a.degree < b.degree;
    return a.id < b.id;
}

static int64 maximumCycles(int64 n) {
    if (n & 1LL) return n * (n * n - 1) / 24;
    return n * (n * n - 4) / 24;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int Q;
    cin >> Q;

    while (Q--) {
        int n;
        int64 M;
        cin >> n >> M;

        /*
        For a tournament with outdegrees d[v],

          cyclic triangles
            = C(n,3) - sum_v C(d[v],2).

        Starting with the transitive score sequence 0,1,...,n-1,
        transfer one degree from a value b to a value a, where b-a >= 2.
        This preserves the tournament-score inequalities and increases the
        number of cyclic triangles by b-a-1.

        Repeatedly transferring between the current minimum and maximum
        degrees keeps every degree value between them present. Therefore,
        when the remaining required increase is smaller than the maximum
        possible transfer, it can be completed by one exact transfer.
        */

        if (M > maximumCycles(n)) {
            cout << "No\n";
            continue;
        }

        vector<int> countDegree(n, 1);
        int low = 0, high = n - 1;
        int64 remaining = M;

        while (remaining > 0) {
            while (low < n && countDegree[low] == 0) ++low;
            while (high >= 0 && countDegree[high] == 0) --high;

            int64 gain = high - low - 1;
            if (gain <= 0) break;

            if (remaining >= gain) {
                --countDegree[low];
                ++countDegree[low + 1];
                --countDegree[high];
                ++countDegree[high - 1];
                remaining -= gain;
            } else {
                int other = low + static_cast<int>(remaining) + 1;

                --countDegree[low];
                ++countDegree[low + 1];
                --countDegree[other];
                ++countDegree[other - 1];
                remaining = 0;
            }
        }

        if (remaining != 0) {
            cout << "No\n";
            continue;
        }

        vector<Node> current;
        current.reserve(n);

        int id = 0;
        for (int degree = 0; degree < n; ++degree) {
            for (int copies = 0; copies < countDegree[degree]; ++copies) {
                current.push_back({degree, id++});
            }
        }

        vector<string> answer(n);
        for (int i = 0; i < n; ++i) answer[i] = string(i, '0');

        auto setWinner = [&](int winner, int loser) {
            if (winner > loser) {
                answer[winner][loser] = '1';
            } else {
                answer[loser][winner] = '0';
            }
        };

        /*
        Realize the score sequence using the tournament version of the
        Havel-Hakimi construction.

        For the vertex v of largest degree d:
        - v beats the d vertices of smallest degree;
        - every other vertex beats v, so its residual degree decreases by 1.

        The two resulting sorted ranges are merged in linear time.
        */
        bool valid = true;

        while (current.size() > 1) {
            int k = static_cast<int>(current.size());
            Node v = current.back();
            current.pop_back();

            int d = v.degree;
            if (d < 0 || d >= k) {
                valid = false;
                break;
            }

            for (int i = 0; i < d; ++i) {
                setWinner(v.id, current[i].id);
            }
            for (int i = d; i < k - 1; ++i) {
                setWinner(current[i].id, v.id);
                --current[i].degree;
                if (current[i].degree < 0) valid = false;
            }

            if (!valid) break;

            vector<Node> merged;
            merged.reserve(k - 1);

            int i = 0;
            int j = d;

            while (i < d && j < k - 1) {
                if (nodeLess(current[i], current[j])) {
                    merged.push_back(current[i++]);
                } else {
                    merged.push_back(current[j++]);
                }
            }
            while (i < d) merged.push_back(current[i++]);
            while (j < k - 1) merged.push_back(current[j++]);

            current.swap(merged);
        }

        if (!valid || (!current.empty() && current[0].degree != 0)) {
            cout << "No\n";
            continue;
        }

        cout << "Yes\n";
        for (int i = 1; i < n; ++i) {
            cout << answer[i] << '\n';
        }
    }

    return 0;
}
