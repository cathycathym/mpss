#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
using i128 = __int128_t;

static const int64 INF = (int64)4e18;

int N;
int64 K;
vector<int64> H, C;

int64 sat_add(int64 a, int64 b) {
    if (a >= INF || b >= INF) return INF;
    i128 z = (i128)a + b;
    return z >= INF ? INF : (int64)z;
}

int64 sat_mul(int64 a, int64 b) {
    i128 z = (i128)a * b;
    return z >= INF ? INF : (int64)z;
}

int64 evaluate_groups(const vector<int64>& altitude, int root) {
    map<int64, vector<int>> groups;
    for (int i = 0; i < N; ++i) groups[altitude[i]].push_back(i);

    if (groups.begin()->second.size() != 1 ||
        groups.begin()->second[0] != root) {
        return INF;
    }

    int largest = 0;
    int64 cheapest = INF;
    int64 answer = 0;
    bool first = true;

    for (auto &entry : groups) {
        const vector<int>& group = entry.second;
        int s = (int)group.size();

        if (first) {
            largest = 1;
            cheapest = C[root];
            first = false;
            continue;
        }

        if (s > largest) {
            answer = sat_add(answer, sat_mul(s - largest, cheapest));
            largest = s;
        }

        for (int v : group) cheapest = min(cheapest, C[v]);
    }

    for (int i = 0; i < N; ++i) {
        answer = sat_add(answer, sat_mul(altitude[i] - H[i], K));
    }
    return answer;
}

int64 solve_subtask1() {
    int64 minimum_height = *min_element(H.begin(), H.end());
    vector<int> candidates;
    for (int i = 0; i < N; ++i) {
        if (H[i] == minimum_height) candidates.push_back(i);
    }

    int64 answer = INF;
    for (int root : candidates) {
        vector<int64> altitude = H;
        for (int v : candidates) {
            if (v != root) altitude[v]++;
        }
        answer = min(answer, evaluate_groups(altitude, root));
    }
    return answer;
}

int64 solve_subtask2() {
    const int root = 0;
    vector<int64> release;
    release.reserve(N - 1);

    for (int i = 1; i < N; ++i) {
        release.push_back(max(H[i], H[root] + 1));
    }
    sort(release.begin(), release.end());

    int64 answer = INF;

    for (int cap = 1; cap <= max(1, N - 1); ++cap) {
        vector<int64> completion(N - 1);
        int64 cost = sat_mul(cap - 1, C[root]);

        for (int i = 0; i < N - 1; ++i) {
            completion[i] = release[i];
            if (i >= cap) {
                completion[i] = max(completion[i], completion[i - cap] + 1);
            }
            cost = sat_add(cost, sat_mul(completion[i] - H[i + 1], K));
        }
        answer = min(answer, cost);
    }
    return answer;
}

struct ExactSolver {
    int n;
    int full;
    vector<int64> h, c;
    vector<int64> min_c;
    map<tuple<int, int64, int>, int64> memo;

    int64 dfs(int mask, int64 last_height, int largest_group) {
        if (mask == full) return 0;

        auto key = make_tuple(mask, last_height, largest_group);
        auto it = memo.find(key);
        if (it != memo.end()) return it->second;

        int remaining = full ^ mask;
        int64 best = INF;

        for (int group = remaining; group; group = (group - 1) & remaining) {
            int size = __builtin_popcount((unsigned)group);
            int64 altitude = last_height + 1;
            int64 raising = 0;

            for (int i = 0; i < n; ++i) {
                if (group >> i & 1) altitude = max(altitude, h[i]);
            }
            for (int i = 0; i < n; ++i) {
                if (group >> i & 1) {
                    raising = sat_add(raising, sat_mul(altitude - h[i], K));
                }
            }

            int64 extension = 0;
            if (size > largest_group) {
                extension = sat_mul(size - largest_group, min_c[mask]);
            }

            int64 suffix = dfs(mask | group, altitude,
                               max(largest_group, size));
            int64 candidate = sat_add(raising, extension);
            candidate = sat_add(candidate, suffix);
            best = min(best, candidate);
        }

        memo[key] = best;
        return best;
    }

    int64 run() {
        n = N;
        h = H;
        c = C;
        full = (1 << n) - 1;
        min_c.assign(1 << n, INF);

        for (int mask = 1; mask <= full; ++mask) {
            int bit = __builtin_ctz((unsigned)mask);
            min_c[mask] = min(c[bit], min_c[mask ^ (1 << bit)]);
        }

        int64 answer = INF;
        for (int root = 0; root < n; ++root) {
            memo.clear();
            answer = min(answer, dfs(1 << root, h[root], 1));
        }
        return answer;
    }
};

int64 heuristic_solution() {
    int64 answer = INF;

    for (int root = 0; root < N; ++root) {
        for (int cap = 1; cap <= max(1, N - 1); ++cap) {
            vector<tuple<int64, int64, int>> jobs;
            for (int i = 0; i < N; ++i) {
                if (i == root) continue;
                int64 release = max(H[i], H[root] + 1);
                jobs.emplace_back(release, C[i], i);
            }
            sort(jobs.begin(), jobs.end());

            priority_queue<pair<int64, int>,
                           vector<pair<int64, int>>,
                           greater<pair<int64, int>>> available;

            int ptr = 0;
            int64 current = H[root] + 1;
            int largest_group = 1;
            int64 cheapest = C[root];
            int64 cost = 0;

            while (ptr < (int)jobs.size() || !available.empty()) {
                if (available.empty() && ptr < (int)jobs.size()) {
                    current = max(current, get<0>(jobs[ptr]));
                }

                while (ptr < (int)jobs.size() &&
                       get<0>(jobs[ptr]) <= current) {
                    available.push({get<1>(jobs[ptr]), get<2>(jobs[ptr])});
                    ++ptr;
                }

                vector<int> group;
                while (!available.empty() && (int)group.size() < cap) {
                    group.push_back(available.top().second);
                    available.pop();
                }

                int s = (int)group.size();
                if (s > largest_group) {
                    cost = sat_add(cost,
                                   sat_mul(s - largest_group, cheapest));
                    largest_group = s;
                }

                for (int v : group) {
                    cost = sat_add(cost, sat_mul(current - H[v], K));
                }
                for (int v : group) cheapest = min(cheapest, C[v]);

                ++current;
            }

            answer = min(answer, cost);
        }
    }
    return answer;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> N >> K;
    H.resize(N);
    C.resize(N);
    for (int i = 0; i < N; ++i) cin >> H[i] >> C[i];

    if (N == 1) {
        cout << 0 << '\n';
        return 0;
    }

    if (N <= 10) {
        ExactSolver solver;
        cout << solver.run() << '\n';
        return 0;
    }

    bool subtask1 = (K >= 100000);
    bool subtask2 = true;

    for (int i = 0; i < N; ++i) {
        subtask1 &= (H[i] <= 300 && C[i] <= 100);
        subtask2 &= (H[0] <= H[i] && C[0] <= C[i] && H[i] <= 300);
    }

    if (subtask1) {
        cout << solve_subtask1() << '\n';
    } else if (subtask2) {
        cout << solve_subtask2() << '\n';
    } else {
        cout << heuristic_solution() << '\n';
    }
    return 0;
}
