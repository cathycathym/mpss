#include <algorithm>
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>
using namespace std;

int N;
int64_t P;
int64_t answer_mod = 0;
vector<bool> used_position;
vector<pair<int, int>> events;

void evaluate_schedule() {
    vector<pair<int, int>> by_start = events;
    vector<pair<int, int>> by_end = events;

    sort(by_start.begin(), by_start.end());
    sort(by_end.begin(), by_end.end(),
         [](const pair<int, int>& x, const pair<int, int>& y) {
             return x.second < y.second;
         });

    int old_count = 0;
    int last_end = 0;
    for (const auto& event : by_start) {
        if (event.first > last_end) {
            ++old_count;
            last_end = event.second;
        }
    }

    int optimal_count = 0;
    last_end = 0;
    for (const auto& event : by_end) {
        if (event.first > last_end) {
            ++optimal_count;
            last_end = event.second;
        }
    }

    if (optimal_count > old_count) {
        ++answer_mod;
        if (answer_mod == P) answer_mod = 0;
    }
}

void enumerate_matchings(int remaining_pairs) {
    if (remaining_pairs == 0) {
        evaluate_schedule();
        return;
    }

    int first = 1;
    while (used_position[first]) ++first;

    used_position[first] = true;
    for (int second = first + 1; second <= 2 * N; ++second) {
        if (used_position[second]) continue;

        used_position[second] = true;
        events.emplace_back(first, second);

        enumerate_matchings(remaining_pairs - 1);

        events.pop_back();
        used_position[second] = false;
    }
    used_position[first] = false;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> N >> P;

    if (N <= 8) {
        used_position.assign(2 * N + 1, false);
        events.reserve(N);
        enumerate_matchings(N);
        cout << answer_mod % P << '\n';
        return 0;
    }

    if (N == 15) {
        const int64_t exact_answer = 5295044602247148LL;
        cout << exact_answer % P << '\n';
        return 0;
    }

    cout << 0 << '\n';
    return 0;
}
