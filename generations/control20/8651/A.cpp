#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int Q;
    cin >> Q;

    while (Q--) {
        int N;
        long long M;
        cin >> N >> M;

        long long maxCycles =
            (long long)(((__int128)N * N * N - N) / 24);

        if (M < 0 || M > maxCycles) {
            cout << "No\n";
            continue;
        }

        // Initially the score sequence is 0,1,...,N-1, corresponding
        // to a transitive tournament with no cyclic triples.
        vector<long long> countDegree(N, 1);

        // Replacing degrees d and d+2 by d+1 and d+1 increases the
        // number of cyclic triples by exactly one.
        set<int> available;

        auto refresh = [&](int d) {
            if (d < 0 || d + 2 >= N) return;
            available.erase(d);
            if (countDegree[d] > 0 && countDegree[d + 2] > 0)
                available.insert(d);
        };

        for (int d = 0; d + 2 < N; ++d)
            refresh(d);

        long long remaining = M;
        bool possible = true;

        while (remaining > 0) {
            if (available.empty()) {
                possible = false;
                break;
            }

            int d = *available.begin();
            long long operations =
                min(remaining, min(countDegree[d], countDegree[d + 2]));

            for (int x = d - 2; x <= d + 2; ++x) {
                if (0 <= x && x + 2 < N)
                    available.erase(x);
            }

            countDegree[d] -= operations;
            countDegree[d + 1] += 2 * operations;
            countDegree[d + 2] -= operations;
            remaining -= operations;

            for (int x = d - 2; x <= d + 2; ++x)
                refresh(x);
        }

        vector<pair<int, int>> vertices;
        vertices.reserve(N);

        if (possible) {
            int id = 0;
            for (int degree = 0; degree < N; ++degree) {
                for (long long k = 0; k < countDegree[degree]; ++k)
                    vertices.push_back({degree, id++});
            }

            if ((int)vertices.size() != N)
                possible = false;
        }

        vector<string> answer(N);
        for (int i = 0; i < N; ++i)
            answer[i] = string(i, '0');

        auto setWinner = [&](int winner, int loser) {
            if (winner > loser)
                answer[winner][loser] = '1';
            else
                answer[loser][winner] = '0';
        };

        // Realize the nondecreasing tournament score sequence.
        if (possible) {
            for (int left = 0; left < N; ++left) {
                int size = N - left;
                int wins = vertices[left].first;

                if (wins < 0 || wins >= size) {
                    possible = false;
                    break;
                }

                int losses = size - 1 - wins;
                int boundary = left + 1 + losses;
                int v = vertices[left].second;

                for (int j = left + 1; j < boundary; ++j) {
                    setWinner(vertices[j].second, v);
                    --vertices[j].first;
                }

                for (int j = boundary; j < N; ++j)
                    setWinner(v, vertices[j].second);
            }
        }

        if (!possible) {
            cout << "No\n";
            continue;
        }

        cout << "Yes\n";
        for (int i = 1; i < N; ++i)
            cout << answer[i] << '\n';
    }

    return 0;
}
