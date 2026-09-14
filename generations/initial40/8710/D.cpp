#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

struct Interval {
    int64 start, finish;

    bool operator<(const Interval& other) const {
        if (start != other.start) return start < other.start;
        return finish < other.finish;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    int64 M;
    cin >> N >> M;

    vector<int64> start(N), length(N);
    vector<int> crossing;

    for (int i = 0; i < N; ++i) {
        int64 end;
        cin >> start[i] >> end;

        length[i] = (end - start[i] + M) % M;
        if (start[i] > end) crossing.push_back(i);
    }

    if (crossing.empty()) {
        cout << -1 << '\n';
        return 0;
    }

    vector<Interval> intervals;
    vector<int64> endpoints;
    intervals.reserve(2 * N);
    endpoints.reserve(2 * N);

    for (int i = 0; i < N; ++i) {
        for (int copy = 0; copy < 2; ++copy) {
            int64 left = start[i] + copy * M;
            int64 right = left + length[i];
            intervals.push_back({left, right});
            endpoints.push_back(right);
        }
    }

    sort(intervals.begin(), intervals.end());
    sort(endpoints.begin(), endpoints.end());
    endpoints.erase(unique(endpoints.begin(), endpoints.end()), endpoints.end());

    int K = static_cast<int>(endpoints.size());
    vector<int> nextState(K, -1);

    int ptr = 0;
    int64 farthest = -1;

    for (int state = 0; state < K; ++state) {
        int64 position = endpoints[state];

        while (ptr < static_cast<int>(intervals.size()) &&
               intervals[ptr].start <= position) {
            farthest = max(farthest, intervals[ptr].finish);
            ++ptr;
        }

        if (farthest > position) {
            nextState[state] = static_cast<int>(
                lower_bound(endpoints.begin(), endpoints.end(), farthest) -
                endpoints.begin()
            );
        }
    }

    int LOG = 1;
    while ((1LL << LOG) <= K) ++LOG;

    vector<vector<int>> jump(LOG, vector<int>(K, -1));
    jump[0] = nextState;

    for (int level = 1; level < LOG; ++level) {
        for (int state = 0; state < K; ++state) {
            int middle = jump[level - 1][state];
            if (middle != -1) {
                jump[level][state] = jump[level - 1][middle];
            }
        }
    }

    int answer = INT_MAX;

    for (int index : crossing) {
        int64 firstEnd = start[index] + length[index];
        int64 target = start[index] + M;

        int state = static_cast<int>(
            lower_bound(endpoints.begin(), endpoints.end(), firstEnd) -
            endpoints.begin()
        );

        int additional = 0;

        for (int level = LOG - 1; level >= 0; --level) {
            int candidate = jump[level][state];
            if (candidate != -1 && endpoints[candidate] < target) {
                state = candidate;
                additional += (1 << level);
            }
        }

        int last = jump[0][state];
        if (last != -1 && endpoints[last] >= target) {
            answer = min(answer, additional + 2);
        }
    }

    cout << (answer == INT_MAX ? -1 : answer) << '\n';
    return 0;
}