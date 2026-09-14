#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <climits>
using namespace std;

using int64 = long long;

struct Requirement {
    int64 left = 0;
    int64 right = 0;
    int64 gap = 0;
    bool valid = false;
};

class ColumnSet {
    map<int64, int> frequency;
    set<int64> columns;
    multiset<int64> gaps;

    void eraseGap(int64 value) {
        auto it = gaps.find(value);
        if (it != gaps.end()) gaps.erase(it);
    }

public:
    void add(int64 x) {
        if (++frequency[x] > 1) return;

        auto nextIt = columns.lower_bound(x);
        auto prevIt = nextIt;

        bool hasNext = nextIt != columns.end();
        bool hasPrev = nextIt != columns.begin();

        if (hasPrev) --prevIt;

        if (hasPrev && hasNext)
            eraseGap(*nextIt - *prevIt - 1);
        if (hasPrev)
            gaps.insert(x - *prevIt - 1);
        if (hasNext)
            gaps.insert(*nextIt - x - 1);

        columns.insert(x);
    }

    void remove(int64 x) {
        auto fit = frequency.find(x);
        if (fit == frequency.end()) return;
        if (--fit->second > 0) return;
        frequency.erase(fit);

        auto it = columns.find(x);
        auto nextIt = next(it);
        bool hasNext = nextIt != columns.end();
        bool hasPrev = it != columns.begin();
        auto prevIt = it;
        if (hasPrev) --prevIt;

        if (hasPrev)
            eraseGap(x - *prevIt - 1);
        if (hasNext)
            eraseGap(*nextIt - x - 1);
        if (hasPrev && hasNext)
            gaps.insert(*nextIt - *prevIt - 1);

        columns.erase(it);
    }

    Requirement requirement(int64 C) const {
        if (columns.empty()) return {};
        Requirement result;
        result.valid = true;
        result.left = *columns.begin() - 1;
        result.right = C - *columns.rbegin();
        result.gap = gaps.empty() ? 0 : *gaps.rbegin();
        return result;
    }
};

struct Point {
    int64 row, col;
};

static void mergeRequirement(Requirement &destination,
                             const Requirement &source) {
    if (!source.valid) {
        destination.valid = false;
        return;
    }
    destination.left = max(destination.left, source.left);
    destination.right = max(destination.right, source.right);
    destination.gap = max(destination.gap, source.gap);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int64 R, C;
    int N;
    cin >> R >> C;
    cin >> N;

    vector<Point> points(N);
    vector<int64> inputRows(N);

    for (int i = 0; i < N; ++i) {
        cin >> points[i].row >> points[i].col;
        inputRows[i] = points[i].row;
    }

    sort(points.begin(), points.end(), [](const Point &a, const Point &b) {
        if (a.row != b.row) return a.row < b.row;
        return a.col < b.col;
    });

    vector<int64> rows;
    vector<vector<int64>> rowColumns;

    for (const Point &p : points) {
        if (rows.empty() || rows.back() != p.row) {
            rows.push_back(p.row);
            rowColumns.push_back({});
        }
        rowColumns.back().push_back(p.col);
    }

    int M = (int)rows.size();

    vector<Requirement> prefix(M), suffix(M);

    {
        ColumnSet current;
        for (int i = 0; i < M; ++i) {
            for (int64 col : rowColumns[i]) current.add(col);
            prefix[i] = current.requirement(C);
        }
    }

    {
        ColumnSet current;
        for (int i = M - 1; i >= 0; --i) {
            for (int64 col : rowColumns[i]) current.add(col);
            suffix[i] = current.requirement(C);
        }
    }

    auto prefixRequirement = [&](int64 maximumRow) -> Requirement {
        int index = int(upper_bound(rows.begin(), rows.end(), maximumRow)
                        - rows.begin()) - 1;
        if (index < 0) return {};
        return prefix[index];
    };

    auto suffixRequirement = [&](int64 minimumRow) -> Requirement {
        int index = int(lower_bound(rows.begin(), rows.end(), minimumRow)
                        - rows.begin());
        if (index >= M) return {};
        return suffix[index];
    };

    vector<int64> candidates;
    int64 maximumH = 2 * (R - 1);

    auto addCandidate = [&](int64 h) {
        if (0 <= h && h <= maximumH) candidates.push_back(h);
    };

    addCandidate(0);
    addCandidate(R - 1);
    addCandidate(maximumH);

    for (int i = 0; i < N; ++i) {
        addCandidate(inputRows[i] - 1);
        addCandidate(R - inputRows[i]);

        for (int j = 0; j < N; ++j) {
            int64 difference = llabs(inputRows[i] - inputRows[j]);
            addCandidate(difference);
            addCandidate(difference - 1);

            int64 combined =
                (inputRows[i] - 1) + (R - inputRows[j]);
            addCandidate(combined);
        }
    }

    sort(candidates.begin(), candidates.end());
    candidates.erase(unique(candidates.begin(), candidates.end()),
                     candidates.end());

    const int64 INF = (1LL << 62);
    int64 answer = INF;

    for (int64 h : candidates) {
        Requirement internal;
        internal.valid = true;

        // Evaluate all intervals [L, L+h] contained in [1,R].
        if (h <= R - 1) {
            int64 maximumL = R - h;
            ColumnSet active;

            int addIndex = 0;
            while (addIndex < M && rows[addIndex] <= 1 + h) {
                for (int64 col : rowColumns[addIndex]) active.add(col);
                ++addIndex;
            }

            int removeIndex = 0;
            Requirement first = active.requirement(C);
            if (!first.valid) {
                internal.valid = false;
            } else {
                mergeRequirement(internal, first);
            }

            while (internal.valid) {
                int64 nextAdd = INF;
                int64 nextRemove = INF;

                if (addIndex < M)
                    nextAdd = rows[addIndex] - h;
                if (removeIndex < M)
                    nextRemove = rows[removeIndex] + 1;

                int64 nextL = min(nextAdd, nextRemove);
                if (nextL > maximumL) break;

                while (removeIndex < M &&
                       rows[removeIndex] + 1 == nextL) {
                    for (int64 col : rowColumns[removeIndex])
                        active.remove(col);
                    ++removeIndex;
                }

                while (addIndex < M &&
                       rows[addIndex] - h == nextL) {
                    for (int64 col : rowColumns[addIndex])
                        active.add(col);
                    ++addIndex;
                }

                Requirement current = active.requirement(C);
                if (!current.valid) {
                    internal.valid = false;
                    break;
                }
                mergeRequirement(internal, current);
            }
        }

        if (!internal.valid) continue;

        int64 lowerU = max<int64>(0, h - (R - 1));
        int64 upperU = min<int64>(R - 1, h);

        vector<int64> verticalSplits;
        verticalSplits.push_back(lowerU);
        verticalSplits.push_back(upperU);

        for (int64 row : inputRows) {
            // This value starts a new top-prefix state.
            int64 topEvent = row - 1;
            if (lowerU <= topEvent && topEvent <= upperU)
                verticalSplits.push_back(topEvent);

            // This value starts a new bottom-suffix state.
            int64 bottomEvent = h + row - R + 1;
            if (lowerU <= bottomEvent && bottomEvent <= upperU)
                verticalSplits.push_back(bottomEvent);
        }

        sort(verticalSplits.begin(), verticalSplits.end());
        verticalSplits.erase(
            unique(verticalSplits.begin(), verticalSplits.end()),
            verticalSplits.end());

        for (int64 up : verticalSplits) {
            int64 down = h - up;

            Requirement top = prefixRequirement(1 + up);
            Requirement bottom = suffixRequirement(R - down);
            if (!top.valid || !bottom.valid) continue;

            Requirement total = internal;
            mergeRequirement(total, top);
            mergeRequirement(total, bottom);

            int64 horizontal =
                max(total.left + total.right, total.gap);
            answer = min(answer, h + horizontal);
        }
    }

    cout << answer << '\n';
    return 0;
}