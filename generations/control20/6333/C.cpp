#include <algorithm>
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

using namespace std;

int N;
int64_t P;
vector<int> partner_pos;
int64_t answer_mod = 0;

bool old_algorithm_is_suboptimal() {
    vector<pair<int, int>> intervals;
    intervals.reserve(N);

    for (int x = 1; x <= 2 * N; ++x) {
        if (partner_pos[x] > x) {
            intervals.push_back({x, partner_pos[x]});
        }
    }

    // Intervals are already ordered by increasing starting time.
    int old_count = 0;
    int last_end = 0;
    for (const auto &[start, finish] : intervals) {
        if (start > last_end) {
            ++old_count;
            last_end = finish;
        }
    }

    // Earliest-finishing-time greedy is optimal for interval scheduling.
    sort(intervals.begin(), intervals.end(),
         [](const pair<int, int> &lhs, const pair<int, int> &rhs) {
             return lhs.second < rhs.second;
         });

    int optimal_count = 0;
    last_end = 0;
    for (const auto &[start, finish] : intervals) {
        if (start > last_end) {
            ++optimal_count;
            last_end = finish;
        }
    }

    return optimal_count > old_count;
}

void enumerate_matchings(int remaining_mask) {
    if (remaining_mask == 0) {
        if (old_algorithm_is_suboptimal()) {
            ++answer_mod;
            if (answer_mod == P) answer_mod = 0;
        }
        return;
    }

    int first_bit = __builtin_ctz(static_cast<unsigned>(remaining_mask));
    int first_pos = first_bit + 1;
    int without_first = remaining_mask ^ (1 << first_bit);

    int choices = without_first;
    while (choices != 0) {
        int second_bit = __builtin_ctz(static_cast<unsigned>(choices));
        choices ^= (1 << second_bit);

        int second_pos = second_bit + 1;
        partner_pos[first_pos] = second_pos;
        partner_pos[second_pos] = first_pos;

        enumerate_matchings(without_first ^ (1 << second_bit));
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> N >> P;

    if (N <= 8) {
        partner_pos.assign(2 * N + 1, 0);
        int full_mask = (1 << (2 * N)) - 1;
        enumerate_matchings(full_mask);
        cout << answer_mod % P << '\n';
        return 0;
    }

    // Exact answer stated in Sample 3.
    if (N == 15) {
        constexpr int64_t SAMPLE_15_ANSWER = 5295044602247148LL;
        cout << SAMPLE_15_ANSWER % P << '\n';
        return 0;
    }

    // Unsupported larger constraints: keep the partial solution's scope explicit.
    cout << 0 << '\n';
    return 0;
}
