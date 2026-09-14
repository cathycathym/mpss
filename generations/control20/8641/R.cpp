#include <bits/stdc++.h>
using namespace std;

using i128 = __int128_t;

static const i128 INF = (i128(1) << 120);

struct Point {
    long long h, c;
};

static void print_i128(i128 x) {
    if (x == 0) {
        cout << "0\n";
        return;
    }
    if (x < 0) {
        cout << '-';
        x = -x;
    }
    string s;
    while (x > 0) {
        s.push_back(char('0' + x % 10));
        x /= 10;
    }
    reverse(s.begin(), s.end());
    cout << s << '\n';
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    long long K;
    cin >> N >> K;

    vector<Point> p(N);
    i128 sumH = 0;
    for (auto &[h, c] : p) {
        cin >> h >> c;
        sumH += h;
    }

    if (N == 1) {
        cout << "0\n";
        return 0;
    }

    vector<int> hotelCandidates;
    for (int r = 0; r < N; ++r) {
        bool dominated = false;
        for (int a = 0; a < N; ++a) {
            if (a == r) continue;
            if (p[a].h <= p[r].h && p[a].c <= p[r].c) {
                if (p[a].h < p[r].h || p[a].c < p[r].c || a < r) {
                    dominated = true;
                    break;
                }
            }
        }
        if (!dominated) hotelCandidates.push_back(r);
    }

    i128 answer = INF;

    for (int root : hotelCandidates) {
        const long long hotelHeight = p[root].h;

        vector<pair<long long, long long>> jobs;
        jobs.reserve(N - 1);

        for (int i = 0; i < N; ++i) {
            if (i == root) continue;
            long long release = max(p[i].h, hotelHeight + 1);
            jobs.push_back({release, p[i].c});
        }

        sort(jobs.begin(), jobs.end());

        struct Event {
            long long time;
            long long priceAfter;
        };

        vector<Event> events;
        long long currentPrice = p[root].c;

        for (int l = 0; l < (int)jobs.size(); ) {
            int r = l;
            long long groupMin = jobs[l].second;
            while (r + 1 < (int)jobs.size() &&
                   jobs[r + 1].first == jobs[l].first) {
                ++r;
                groupMin = min(groupMin, jobs[r].second);
            }

            if (groupMin < currentPrice) {
                currentPrice = groupMin;
                events.push_back({jobs[l].first, currentPrice});
            }

            l = r + 1;
        }

        // State immediately after processing the current boundary:
        // dp[backlog][capacity].
        vector<vector<i128>> dp(
            N + 1, vector<i128>(N + 1, INF)
        );
        dp[0][1] = i128(K) * hotelHeight;

        long long boundary = hotelHeight;
        long long price = p[root].c;
        int jobBegin = 0;

        auto advance_interval =
            [&](int initialBacklog, int capacity,
                const vector<long long>& releases,
                long long endTime,
                bool finiteEnd) -> pair<int, i128> {

                int q = initialBacklog;
                int m = (int)releases.size();
                vector<long long> completion(m);

                i128 completionSum = 0;
                int completed = 0;

                // Initially backlogged jobs all have release <= boundary.
                int initiallyCompleted = q;
                if (finiteEnd) {
                    long long availableLevels = max(0LL, endTime - boundary);
                    initiallyCompleted =
                        min(q, int(min<long long>(
                            q, availableLevels * capacity
                        )));
                }

                if (initiallyCompleted > 0) {
                    int full = initiallyCompleted / capacity;
                    int rem = initiallyCompleted % capacity;

                    completionSum +=
                        i128(capacity) *
                        (i128(full) * boundary +
                         i128(full) * (full + 1) / 2);

                    if (rem) {
                        completionSum +=
                            i128(rem) * (boundary + full + 1);
                    }
                }
                completed += initiallyCompleted;

                for (int a = 0; a < m; ++a) {
                    int globalIndex = q + a;
                    long long finish;

                    if (globalIndex < capacity) {
                        finish = releases[a];
                    } else {
                        int prevIndex = globalIndex - capacity;
                        long long previousFinish;

                        if (prevIndex < q) {
                            previousFinish =
                                boundary + 1 + prevIndex / capacity;
                        } else {
                            previousFinish = completion[prevIndex - q];
                        }

                        finish = max(releases[a], previousFinish + 1);
                    }

                    completion[a] = finish;

                    if (!finiteEnd || finish <= endTime) {
                        ++completed;
                        completionSum += finish;
                    }
                }

                int total = q + m;
                return {total - completed, completionSum};
            };

        for (const Event &event : events) {
            vector<long long> releases;
            while (jobBegin < (int)jobs.size() &&
                   jobs[jobBegin].first <= event.time) {
                if (jobs[jobBegin].first > boundary)
                    releases.push_back(jobs[jobBegin].first);
                ++jobBegin;
            }

            vector<vector<i128>> afterPurchase(
                N + 1, vector<i128>(N + 1, INF)
            );

            for (int q = 0; q <= N; ++q) {
                i128 prefixBest = INF;

                for (int newCap = 1; newCap <= N; ++newCap) {
                    if (dp[q][newCap] < INF) {
                        prefixBest = min(
                            prefixBest,
                            dp[q][newCap] -
                            i128(newCap) * price
                        );
                    }

                    if (prefixBest < INF) {
                        afterPurchase[q][newCap] =
                            prefixBest + i128(newCap) * price;
                    }
                }
            }

            vector<vector<i128>> next(
                N + 1, vector<i128>(N + 1, INF)
            );

            for (int q = 0; q <= N; ++q) {
                for (int cap = 1; cap <= N; ++cap) {
                    if (afterPurchase[q][cap] == INF) continue;

                    auto [newBacklog, completionSum] =
                        advance_interval(
                            q, cap, releases, event.time, true
                        );

                    i128 candidate =
                        afterPurchase[q][cap] +
                        i128(K) * completionSum;

                    next[newBacklog][cap] =
                        min(next[newBacklog][cap], candidate);
                }
            }

            dp.swap(next);
            boundary = event.time;
            price = event.priceAfter;
        }

        vector<long long> finalReleases;
        while (jobBegin < (int)jobs.size()) {
            finalReleases.push_back(jobs[jobBegin].first);
            ++jobBegin;
        }

        for (int q = 0; q <= N; ++q) {
            i128 prefixBest = INF;

            for (int finalCap = 1; finalCap <= N; ++finalCap) {
                if (dp[q][finalCap] < INF) {
                    prefixBest = min(
                        prefixBest,
                        dp[q][finalCap] -
                        i128(finalCap) * price
                    );
                }

                if (prefixBest == INF) continue;

                i128 stateCost =
                    prefixBest + i128(finalCap) * price;

                auto [remaining, completionSum] =
                    advance_interval(
                        q, finalCap, finalReleases, 0, false
                    );

                if (remaining == 0) {
                    answer = min(
                        answer,
                        stateCost +
                        i128(K) * completionSum -
                        i128(K) * sumH
                    );
                }
            }
        }
    }

    print_i128(answer);
    return 0;
}
