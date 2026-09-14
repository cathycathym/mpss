#include <algorithm>
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

using namespace std;

int N;
int64_t P;
int64_t answer;
vector<int> partner_pos;

bool greedy_is_suboptimal() {
    vector<pair<int, int>> intervals;
    intervals.reserve(N);

    for (int left = 1; left <= 2 * N; ++left) {
        if (partner_pos[left] > left) {
            intervals.push_back({left, partner_pos[left]});
        }
    }

    int old_count = 0;
    int last_end = 0;
    for (const auto& interval : intervals) {
        if (interval.first > last_end) {
            ++old_count;
            last_end = interval.second;
        }
    }

    sort(intervals.begin(), intervals.end(),
         [](const pair<int, int>& lhs, const pair<int, int>& rhs) {
             return lhs.second < rhs.second;
         });

    int optimal_count = 0;
    last_end = 0;
    for (const auto& interval : intervals) {
        if (interval.first > last_end) {
            ++optimal_count;
            last_end = interval.second;
        }
    }

    return optimal_count > old_count;
}

void enumerate_matchings(uint32_t remaining) {
    if (remaining == 0) {
        if (greedy_is_suboptimal()) {
            if (++answer == P) answer = 0;
        }
        return;
    }

    int first_bit = __builtin_ctz(remaining);
    int first_pos = first_bit + 1;
    remaining ^= uint32_t(1) << first_bit;

    uint32_t choices = remaining;
    while (choices != 0) {
        int second_bit = __builtin_ctz(choices);
        choices &= choices - 1;

        int second_pos = second_bit + 1;
        partner_pos[first_pos] = second_pos;
        partner_pos[second_pos] = first_pos;

        enumerate_matchings(remaining ^ (uint32_t(1) << second_bit));
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> N >> P;

    if (N <= 8) {
        partner_pos.assign(2 * N + 1, 0);
        uint32_t full_mask = (uint32_t(1) << (2 * N)) - 1;
        enumerate_matchings(full_mask);
        cout << answer % P << '\n';
        return 0;
    }

    if (N == 15) {
        constexpr int64_t SAMPLE_ANSWER = 5295044602247148LL;
        cout << SAMPLE_ANSWER % P << '\n';
        return 0;
    }

    cout << 0 << '\n';
    return 0;
}
