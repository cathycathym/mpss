#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

struct Vertex {
    int degree;
    int id;
};

static bool lessVertex(const Vertex& a, const Vertex& b) {
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

        const int64 n = N;
        const int64 maximumCycles =
            (N & 1) ? n * (n * n - 1) / 24
                    : n * (n * n - 4) / 24;

        if (M < 0 || M > maximumCycles) {
            cout << "No\n";
            continue;
        }

        // Initially the degrees are 0,1,...,N-1, giving a transitive
        // tournament with no cyclic triples.
        vector<int> countDegree(N, 1);

        int low = 0;
        int high = N - 1;
        int64 remaining = M;

        while (remaining > 0) {
            while (low < N && countDegree[low] == 0) ++low;
            while (high >= 0 && countDegree[high] == 0) --high;

            int64 gain = high - low - 1;

            if (gain <= remaining) {
                --countDegree[low];
                ++countDegree[low + 1];
                --countDegree[high];
                ++countDegree[high - 1];
                remaining -= gain;
            } else {
                int gap = static_cast<int>(remaining + 1);
                int a = low;

                while (a + gap <= high &&
                       (countDegree[a] == 0 ||
                        countDegree[a + gap] == 0)) {
                    ++a;
                }

                if (a + gap > high) break;

                int b = a + gap;
                --countDegree[a];
                ++countDegree[a + 1];
                --countDegree[b];
                ++countDegree[b - 1];
                remaining = 0;
            }
        }

        if (remaining != 0) {
            cout << "No\n";
            continue;
        }

        vector<Vertex> current;
        current.reserve(N);

        int nextId = 0;
        for (int degree = 0; degree < N; ++degree) {
            for (int k = 0; k < countDegree[degree]; ++k) {
                current.push_back({degree, nextId++});
            }
        }

        vector<string> answer(N);
        for (int i = 0; i < N; ++i) {
            answer[i] = string(i, '0');
        }

        auto setWinner = [&](int winner, int loser) {
            if (winner > loser) {
                answer[winner][loser] = '1';
            } else {
                answer[loser][winner] = '0';
            }
        };

        bool possible = (static_cast<int>(current.size()) == N);

        while (possible && current.size() > 1) {
            Vertex v = current.back();
            current.pop_back();

            int size = static_cast<int>(current.size());
            int beaten = v.degree;

            if (beaten < 0 || beaten > size) {
                possible = false;
                break;
            }

            vector<Vertex> unchanged;
            vector<Vertex> decreased;
            unchanged.reserve(beaten);
            decreased.reserve(size - beaten);

            for (int i = 0; i < beaten; ++i) {
                Vertex u = current[i];
                setWinner(v.id, u.id);
                unchanged.push_back(u);
            }

            for (int i = beaten; i < size; ++i) {
                Vertex u = current[i];
                setWinner(u.id, v.id);
                --u.degree;

                if (u.degree < 0) {
                    possible = false;
                    break;
                }
                decreased.push_back(u);
            }

            if (!possible) break;

            vector<Vertex> merged;
            merged.reserve(size);

            size_t i = 0, j = 0;
            while (i < unchanged.size() || j < decreased.size()) {
                if (j == decreased.size() ||
                    (i < unchanged.size() &&
                     lessVertex(unchanged[i], decreased[j]))) {
                    merged.push_back(unchanged[i++]);
                } else {
                    merged.push_back(decreased[j++]);
                }
            }

            current.swap(merged);
        }

        if (possible && !current.empty() && current[0].degree != 0) {
            possible = false;
        }

        if (!possible) {
            cout << "No\n";
            continue;
        }

        cout << "Yes\n";
        for (int i = 1; i < N; ++i) {
            cout << answer[i] << '\n';
        }
    }

    return 0;
}
