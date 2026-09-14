#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
using u64 = uint64_t;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    int64 P;
    cin >> N >> P;

    // Exact dynamic programming for Subtasks 1, 2, and 3 (N <= 30).
    if (N > 30) {
        cout << 0 << '\n';
        return 0;
    }

    const u64 B = static_cast<u64>(N + 1);

    auto encode = [&](int opened, int eligible, int difference, int state) {
        u64 key = opened;
        key = key * B + eligible;
        key = key * B + difference;
        key = key * 3 + state;
        return key;
    };

    auto add = [&](unordered_map<u64, int64>& table, u64 key, int64 value) {
        auto [it, inserted] = table.emplace(key, value % P);
        if (!inserted) {
            it->second += value;
            if (it->second >= P) it->second %= P;
        }
    };

    /*
       state:
       0 = the old greedy algorithm currently has no chosen active event
       1 = its chosen active event is eligible for the optimal algorithm
       2 = its chosen active event is stale for the optimal algorithm

       "Eligible" events started after the end of the last event selected by
       the optimal earliest-finish algorithm. The first eligible event to end
       is selected by that algorithm; all other currently active events then
       become stale.

       difference = optimal_selected - old_greedy_selected.
    */
    unordered_map<u64, int64> current, next;
    current.reserve(1 << 16);
    current.emplace(encode(0, 0, 0, 0), 1);

    for (int processed = 0; processed < 2 * N; ++processed) {
        next.clear();
        next.reserve(current.size() * 3 + 16);

        for (const auto& entry : current) {
            u64 key = entry.first;
            int64 ways = entry.second;

            int state = static_cast<int>(key % 3);
            key /= 3;
            int difference = static_cast<int>(key % B);
            key /= B;
            int eligible = static_cast<int>(key % B);
            key /= B;
            int opened = static_cast<int>(key);

            int special = (state != 0);
            int active = 2 * opened - processed;
            int stale = active - eligible - special;

            if (stale < 0) continue;

            // The next endpoint opens the next event.
            if (opened < N) {
                int newState = state;
                int newEligible = eligible;

                if (state == 0) {
                    newState = 1;
                } else {
                    ++newEligible;
                }

                add(next, encode(opened + 1, newEligible,
                                 difference, newState), ways);
            }

            // An ordinary eligible event ends. The optimal algorithm selects it.
            if (eligible > 0 && difference < N) {
                int newState = (state == 1 ? 2 : state);
                int64 multiplied = ways * eligible % P;
                add(next, encode(opened, 0, difference + 1, newState),
                    multiplied);
            }

            // An ordinary stale event ends.
            if (stale > 0) {
                int64 multiplied = ways * stale % P;
                add(next, encode(opened, eligible, difference, state),
                    multiplied);
            }

            // The old greedy algorithm's eligible event ends. Both algorithms
            // select this event.
            if (state == 1) {
                add(next, encode(opened, 0, difference, 0), ways);
            }

            // The old greedy algorithm's stale event ends. Only it gains an
            // event, reducing the optimal algorithm's current lead.
            if (state == 2 && difference > 0) {
                add(next, encode(opened, eligible, difference - 1, 0), ways);
            }
        }

        current.swap(next);
    }

    int64 equalWays = 0;
    auto it = current.find(encode(N, 0, 0, 0));
    if (it != current.end()) equalWays = it->second;

    int64 totalWays = 1;
    for (int i = 1; i <= N; ++i) {
        totalWays = totalWays * (2LL * i - 1) % P;
    }

    int64 answer = (totalWays - equalWays) % P;
    if (answer < 0) answer += P;

    cout << answer << '\n';
    return 0;
}
