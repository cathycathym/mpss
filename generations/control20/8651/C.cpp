#include <algorithm>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

using namespace std;

using int64 = long long;

struct Item {
    int degree;
    int id;
};

static bool itemLess(const Item& a, const Item& b) {
    if (a.degree != b.degree) return a.degree < b.degree;
    return a.id < b.id;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int Q;
    cin >> Q;

    while (Q--) {
        int N;
        int64 M;
        cin >> N >> M;

        int64 n = N;
        int64 maximumCycles = (n * (n * n - 1)) / 24;

        if (M > maximumCycles) {
            cout << "No\n";
            continue;
        }

        // cnt[d] is the number of vertices whose desired outdegree is d.
        vector<int> cnt(N, 1);

        int low = 0;
        int high = N - 1;
        int64 remaining = M;

        while (remaining > 0) {
            while (low < N && cnt[low] == 0) ++low;
            while (high >= 0 && cnt[high] == 0) --high;

            int64 increase = high - low - 1;

            if (increase <= remaining) {
                // Transfer one win from a maximum-degree vertex to a
                // minimum-degree vertex.
                --cnt[low];
                ++cnt[low + 1];
                --cnt[high];
                ++cnt[high - 1];
                remaining -= increase;
            } else {
                // All degrees in [low, high] are currently occupied.
                // Find two whose difference is remaining + 1.
                int gap = static_cast<int>(remaining + 1);
                int a = low;

                while (a + gap <= high &&
                       (cnt[a] == 0 || cnt[a + gap] == 0)) {
                    ++a;
                }

                int b = a + gap;

                --cnt[a];
                ++cnt[a + 1];
                --cnt[b];
                ++cnt[b - 1];
                remaining = 0;
            }
        }

        vector<Item> current;
        current.reserve(N);

        int nextId = 0;
        for (int degree = 0; degree < N; ++degree) {
            for (int k = 0; k < cnt[degree]; ++k) {
                current.push_back({degree, nextId++});
            }
        }

        vector<string> result(N, string(N, '?'));
        for (int i = 0; i < N; ++i) result[i][i] = '-';

        // Realize the tournament score sequence.
        while (current.size() > 1) {
            Item v = current.back();
            current.pop_back();

            int m = static_cast<int>(current.size());
            int beaten = v.degree;

            vector<Item> left;
            vector<Item> right;
            left.reserve(beaten);
            right.reserve(m - beaten);

            // v beats the vertices with the smallest scores.
            for (int i = 0; i < beaten; ++i) {
                Item u = current[i];
                result[v.id][u.id] = '1';
                result[u.id][v.id] = '0';
                left.push_back(u);
            }

            // The remaining vertices beat v, so their residual scores drop.
            for (int i = beaten; i < m; ++i) {
                Item u = current[i];
                result[v.id][u.id] = '0';
                result[u.id][v.id] = '1';
                --u.degree;
                right.push_back(u);
            }

            // Both parts are sorted; merge them in linear time.
            vector<Item> merged;
            merged.reserve(m);

            size_t i = 0, j = 0;
            while (i < left.size() || j < right.size()) {
                if (j == right.size() ||
                    (i < left.size() && itemLess(left[i], right[j]))) {
                    merged.push_back(left[i++]);
                } else {
                    merged.push_back(right[j++]);
                }
            }

            current.swap(merged);
        }

        cout << "Yes\n";
        for (int i = 1; i < N; ++i) {
            for (int j = 0; j < i; ++j) {
                cout << result[i][j];
            }
            cout << '\n';
        }
    }

    return 0;
}
