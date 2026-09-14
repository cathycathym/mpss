#include <iostream>
#include <vector>
#include <algorithm>
#include <deque>
#include <limits>

using namespace std;

using int64 = long long;

struct Point {
    int64 row, col;
};

struct Info {
    int64 leftNeed = 0;
    int64 rightNeed = 0;
    int64 gapNeed = 0;
};

class MaxQueue {
private:
    deque<pair<int, int64>> q;

public:
    void clear() {
        q.clear();
    }

    void push(int index, int64 value) {
        while (!q.empty() && q.back().second <= value) {
            q.pop_back();
        }
        q.emplace_back(index, value);
    }

    void removeBefore(int index) {
        while (!q.empty() && q.front().first < index) {
            q.pop_front();
        }
    }

    int64 maximum() const {
        return q.front().second;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int64 R, C;
    int N;
    cin >> R >> C;
    cin >> N;

    vector<Point> seeds(N);
    for (Point &p : seeds) {
        cin >> p.row >> p.col;
    }

    sort(seeds.begin(), seeds.end(), [](const Point &a, const Point &b) {
        if (a.row != b.row) return a.row < b.row;
        return a.col < b.col;
    });

    // horizontal[l][r] stores the horizontal requirements for
    // exactly the row-sorted seeds with indices l...r.
    vector<vector<Info>> horizontal(N, vector<Info>(N));

    for (int l = 0; l < N; ++l) {
        vector<int64> columns;

        for (int r = l; r < N; ++r) {
            auto pos = upper_bound(columns.begin(), columns.end(), seeds[r].col);
            columns.insert(pos, seeds[r].col);

            Info current;
            current.leftNeed = columns.front() - 1;
            current.rightNeed = C - columns.back();

            for (int i = 1; i < static_cast<int>(columns.size()); ++i) {
                current.gapNeed = max(
                    current.gapNeed,
                    columns[i] - columns[i - 1] - 1
                );
            }

            horizontal[l][r] = current;
        }
    }

    // Smallest V for which the vertical intervals can cover R consecutive rows.
    int64 lowerBound =
        (seeds.front().row - 1) + (R - seeds.back().row);

    for (int i = 0; i + 1 < N; ++i) {
        lowerBound = max(
            lowerBound,
            seeds[i + 1].row - seeds[i].row - 1
        );
    }

    vector<int64> candidates;
    candidates.reserve(2 * N * N);

    // A field-window boundary meets an interval boundary.
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            candidates.push_back(
                (seeds[i].row - 1) + (R - seeds[j].row)
            );
        }
    }

    // Two seed-interval boundaries meet.
    for (int i = 0; i < N; ++i) {
        for (int j = i; j < N; ++j) {
            candidates.push_back(
                seeds[j].row - seeds[i].row - 1
            );
        }
    }

    sort(candidates.begin(), candidates.end());
    candidates.erase(
        unique(candidates.begin(), candidates.end()),
        candidates.end()
    );

    const int64 INF = numeric_limits<int64>::max() / 4;

    auto horizontalCost = [&](int64 verticalCost) -> int64 {
        // Events of intervals [row, row + verticalCost].
        // Half-open representation: [row, row + verticalCost + 1).
        vector<int64> events;
        events.reserve(2 * N);

        for (const Point &p : seeds) {
            events.push_back(p.row);
        }
        for (const Point &p : seeds) {
            events.push_back(p.row + verticalCost + 1);
        }

        // Both halves are sorted because seeds are sorted by row.
        inplace_merge(events.begin(), events.begin() + N, events.end());
        events.erase(unique(events.begin(), events.end()), events.end());

        int stripCount = static_cast<int>(events.size()) - 1;
        vector<Info> strips(stripCount);

        int firstActive = 0;
        int lastActive = -1;

        for (int k = 0; k < stripCount; ++k) {
            int64 row = events[k];

            while (firstActive < N &&
                   seeds[firstActive].row + verticalCost + 1 <= row) {
                ++firstActive;
            }

            while (lastActive + 1 < N &&
                   seeds[lastActive + 1].row <= row) {
                ++lastActive;
            }

            if (firstActive > lastActive) {
                return INF;
            }

            strips[k] = horizontal[firstActive][lastActive];
        }

        MaxQueue leftQueue, rightQueue, gapQueue;
        int rightStrip = -1;
        int64 result = INF;

        for (int leftStrip = 0; leftStrip < stripCount; ++leftStrip) {
            int64 lastRow = events[leftStrip] + R - 1;

            // The complete field window must stay inside the covered range.
            if (lastRow >= events.back()) {
                break;
            }

            leftQueue.removeBefore(leftStrip);
            rightQueue.removeBefore(leftStrip);
            gapQueue.removeBefore(leftStrip);

            while (rightStrip + 1 < stripCount &&
                   events[rightStrip + 1] <= lastRow) {
                ++rightStrip;
                leftQueue.push(
                    rightStrip,
                    strips[rightStrip].leftNeed
                );
                rightQueue.push(
                    rightStrip,
                    strips[rightStrip].rightNeed
                );
                gapQueue.push(
                    rightStrip,
                    strips[rightStrip].gapNeed
                );
            }

            int64 needLeft = leftQueue.maximum();
            int64 needRight = rightQueue.maximum();
            int64 needGap = gapQueue.maximum();

            result = min(
                result,
                max(needGap, needLeft + needRight)
            );
        }

        return result;
    };

    // Expanding from any single initial seed always gives this upper bound.
    int64 answer = R + C - 2;

    for (int64 verticalCost : candidates) {
        if (verticalCost < lowerBound || verticalCost >= answer) {
            continue;
        }

        int64 hCost = horizontalCost(verticalCost);
        if (hCost != INF) {
            answer = min(answer, verticalCost + hCost);
        }
    }

    cout << answer << '\n';
    return 0;
}