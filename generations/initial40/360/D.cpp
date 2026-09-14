#include <algorithm>
#include <deque>
#include <iostream>
#include <limits>
#include <vector>

using namespace std;

using int64 = long long;

struct Point {
    int64 row, col;
};

struct Info {
    int64 left = 0;
    int64 right = 0;
    int64 gap = 0;
};

class MaxQueue {
    deque<pair<int, int64>> data;

public:
    void push(int index, int64 value) {
        while (!data.empty() && data.back().second <= value)
            data.pop_back();
        data.emplace_back(index, value);
    }

    void removeBefore(int index) {
        while (!data.empty() && data.front().first < index)
            data.pop_front();
    }

    int64 maximum() const {
        return data.front().second;
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
    for (Point &p : seeds)
        cin >> p.row >> p.col;

    sort(seeds.begin(), seeds.end(), [](const Point &a, const Point &b) {
        if (a.row != b.row) return a.row < b.row;
        return a.col < b.col;
    });

    vector<int64> row(N);
    for (int i = 0; i < N; ++i)
        row[i] = seeds[i].row;

    /*
     * requirement[l][r] is the minimum horizontal information needed
     * when exactly seeds l..r are vertically able to reach a row.
     */
    vector<vector<Info>> requirement(N, vector<Info>(N));

    for (int l = 0; l < N; ++l) {
        vector<int64> columns;

        for (int r = l; r < N; ++r) {
            auto it = upper_bound(columns.begin(), columns.end(), seeds[r].col);
            columns.insert(it, seeds[r].col);

            Info cur;
            cur.left = columns.front() - 1;
            cur.right = C - columns.back();

            for (int i = 1; i < static_cast<int>(columns.size()); ++i)
                cur.gap = max(cur.gap, columns[i] - columns[i - 1] - 1);

            requirement[l][r] = cur;
        }
    }

    /*
     * A vertical budget h is feasible only if the translated intervals
     * [row_i, row_i+h] can cover R consecutive integer positions.
     */
    int64 verticalLowerBound =
        (seeds.front().row - 1) + (R - seeds.back().row);

    for (int i = 0; i + 1 < N; ++i) {
        verticalLowerBound = max(
            verticalLowerBound,
            seeds[i + 1].row - seeds[i].row - 1
        );
    }

    const int64 maximumVertical = 2 * (R - 1);
    vector<int64> verticalCandidates;

    auto addCandidate = [&](int64 value) {
        if (0 <= value && value <= maximumVertical)
            verticalCandidates.push_back(value);
    };

    addCandidate(0);
    addCandidate(R - 1);
    addCandidate(maximumVertical);

    for (int i = 0; i < N; ++i) {
        addCandidate(seeds[i].row - 1);
        addCandidate(R - seeds[i].row);

        for (int j = 0; j < N; ++j) {
            int64 difference = llabs(seeds[i].row - seeds[j].row);
            addCandidate(difference - 1);
            addCandidate(difference);

            addCandidate(
                (seeds[i].row - 1) + (R - seeds[j].row)
            );
        }
    }

    sort(verticalCandidates.begin(), verticalCandidates.end());
    verticalCandidates.erase(
        unique(verticalCandidates.begin(), verticalCandidates.end()),
        verticalCandidates.end()
    );

    const int64 INF = numeric_limits<int64>::max() / 4;

    auto horizontalCost = [&](int64 h) -> int64 {
        /*
         * After translating target row x to y=x+up, a seed at row s
         * is active exactly on the integer interval [s,s+h].
         * State changes occur at s and s+h+1.
         */
        vector<int64> events;
        events.reserve(2 * N);

        int first = 0;
        int second = 0;

        while (first < N || second < N) {
            int64 value;

            if (second == N ||
                (first < N && row[first] <= row[second] + h + 1)) {
                value = row[first++];
            } else {
                value = row[second++] + h + 1;
            }

            if (events.empty() || events.back() != value)
                events.push_back(value);
        }

        int stripCount = static_cast<int>(events.size()) - 1;
        if (stripCount <= 0)
            return INF;

        vector<Info> strips(stripCount);
        vector<char> invalid(stripCount, false);

        for (int k = 0; k < stripCount; ++k) {
            int64 y = events[k];

            int l = static_cast<int>(
                lower_bound(row.begin(), row.end(), y - h) - row.begin()
            );
            int r = static_cast<int>(
                upper_bound(row.begin(), row.end(), y) - row.begin()
            ) - 1;

            if (l > r)
                invalid[k] = true;
            else
                strips[k] = requirement[l][r];
        }

        /*
         * The translated field begins at a=1+up, so 1<=a<=h+1.
         * Its set of intersected strips changes only when a or a+R-1
         * crosses an event.
         */
        vector<int64> starts;
        starts.reserve(2 * events.size() + 2);
        starts.push_back(1);

        size_t i = 0, j = 0;
        int64 lastAdded = 1;

        while (i < events.size() || j < events.size()) {
            int64 x;

            if (j == events.size() ||
                (i < events.size() &&
                 events[i] <= events[j] - R + 1)) {
                x = events[i++];
            } else {
                x = events[j++] - R + 1;
            }

            if (1 < x && x < h + 1 && x != lastAdded) {
                starts.push_back(x);
                lastAdded = x;
            }
        }

        if (h + 1 != starts.back())
            starts.push_back(h + 1);

        MaxQueue leftQueue, rightQueue, gapQueue, invalidQueue;
        int addedThrough = -1;
        int64 result = INF;

        for (int64 a : starts) {
            int64 b = a + R - 1;

            if (a < events.front())
                continue;
            if (b >= events.back())
                break;

            int leftStrip = static_cast<int>(
                upper_bound(events.begin(), events.end(), a)
                - events.begin()
            ) - 1;

            int rightStrip = static_cast<int>(
                upper_bound(events.begin(), events.end(), b)
                - events.begin()
            ) - 1;

            leftQueue.removeBefore(leftStrip);
            rightQueue.removeBefore(leftStrip);
            gapQueue.removeBefore(leftStrip);
            invalidQueue.removeBefore(leftStrip);

            while (addedThrough < rightStrip) {
                ++addedThrough;
                leftQueue.push(addedThrough, strips[addedThrough].left);
                rightQueue.push(addedThrough, strips[addedThrough].right);
                gapQueue.push(addedThrough, strips[addedThrough].gap);
                invalidQueue.push(
                    addedThrough,
                    invalid[addedThrough] ? 1 : 0
                );
            }

            if (invalidQueue.maximum() != 0)
                continue;

            int64 needLeft = leftQueue.maximum();
            int64 needRight = rightQueue.maximum();
            int64 needGap = gapQueue.maximum();

            result = min(result, max(needLeft + needRight, needGap));
        }

        return result;
    };

    // Expanding from any one initial seed gives this valid upper bound.
    int64 answer = R + C - 2;

    for (int64 vertical : verticalCandidates) {
        if (vertical < verticalLowerBound || vertical >= answer)
            continue;

        int64 horizontal = horizontalCost(vertical);
        if (horizontal != INF)
            answer = min(answer, vertical + horizontal);
    }

    cout << answer << '\n';
    return 0;
}