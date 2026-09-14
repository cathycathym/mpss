#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

struct Interval {
    int left, right;
};

class Solver {
    int n, split;
    vector<int64> leftArm, rightAscending;
    vector<int64> windowStart, complementStart;
    vector<int64> windowMiddle, complementMiddle;

    static bool intersects(const vector<Interval>& a,
                           const vector<Interval>& b) {
        for (const auto& x : a) {
            for (const auto& y : b) {
                if (max(x.left, y.left) <= min(x.right, y.right))
                    return true;
            }
        }
        return false;
    }

    vector<Interval> feasibleIntervals(const vector<int64>& pots,
                                       int64 limit,
                                       bool complement) const {
        vector<Interval> result;

        for (int phase = 1; phase <= 2; ++phase) {
            int updates = (phase == 1 ? split : n - split);
            int basePosition = (phase == 1 ? 1 : split + 1);

            const vector<int64>& start =
                phase == 1
                    ? (complement ? complementStart : windowStart)
                    : (complement ? complementMiddle : windowMiddle);

            bool lowImproves =
                (phase == 1) ? !complement : complement;

            int minimumTime = 0;
            int maximumTime = updates;
            bool possible = true;

            int startLessL = 0;
            int leftLessL = 0;
            int rightLessL = 0;

            int startLeU = 0;
            int leftLeU = 0;
            int rightLeU = 0;

            auto processConstraint =
                [&](int badness, int crossingCount, int firstCrossingTime,
                    bool improves) {
                    if (!possible) return;

                    if (improves) {
                        if (badness <= 0) return;
                        if (crossingCount < badness) {
                            possible = false;
                            return;
                        }
                        minimumTime =
                            max(minimumTime,
                                firstCrossingTime + badness - 1);
                    } else {
                        if (badness > 0) {
                            possible = false;
                            return;
                        }

                        int allowance = -badness;
                        if (crossingCount > allowance) {
                            maximumTime =
                                min(maximumTime,
                                    firstCrossingTime + allowance - 1);
                        }
                    }
                };

            for (int k = 0; k < n && possible; ++k) {
                int64 lower = pots[k] - limit;
                int64 upper = pots[k] + limit;

                while (startLessL < n && start[startLessL] < lower)
                    ++startLessL;
                while (leftLessL < n && leftArm[leftLessL] < lower)
                    ++leftLessL;
                while (rightLessL < n &&
                       rightAscending[rightLessL] < lower)
                    ++rightLessL;

                while (startLeU < n && start[startLeU] <= upper)
                    ++startLeU;
                while (leftLeU < n && leftArm[leftLeU] <= upper)
                    ++leftLeU;
                while (rightLeU < n &&
                       rightAscending[rightLeU] <= upper)
                    ++rightLeU;

                int lowBadness = startLessL - k;
                int highBadness = (k + 1) - startLeU;

                int lowCrossings, highCrossings;
                int lowFirstTime, highFirstTime;

                if (phase == 1) {
                    int rightAtLeastLower = n - rightLessL;
                    int rightGreaterUpper = n - rightLeU;

                    lowCrossings =
                        min({split, leftLessL, rightAtLeastLower});
                    highCrossings =
                        min({split, leftLeU, rightGreaterUpper});

                    lowFirstTime = highFirstTime = 1;
                } else {
                    int rightAtLeastLower = n - rightLessL;
                    int rightGreaterUpper = n - rightLeU;

                    int lowFirstIndex =
                        max({split + 1,
                             leftLessL + 1,
                             rightAtLeastLower + 1});
                    int highFirstIndex =
                        max({split + 1,
                             leftLeU + 1,
                             rightGreaterUpper + 1});

                    lowCrossings =
                        lowFirstIndex <= n ? n - lowFirstIndex + 1 : 0;
                    highCrossings =
                        highFirstIndex <= n ? n - highFirstIndex + 1 : 0;

                    lowFirstTime = lowFirstIndex - split;
                    highFirstTime = highFirstIndex - split;
                }

                processConstraint(lowBadness, lowCrossings,
                                  lowFirstTime, lowImproves);
                processConstraint(highBadness, highCrossings,
                                  highFirstTime, !lowImproves);
            }

            if (possible && minimumTime <= maximumTime) {
                result.push_back(
                    {basePosition + minimumTime,
                     basePosition + maximumTime});
            }
        }

        return result;
    }

public:
    Solver(int n_, const vector<int64>& a) : n(n_) {
        leftArm.assign(a.begin(), a.begin() + n);

        vector<int64> rightDescending(a.begin() + n, a.end());
        rightAscending = rightDescending;
        reverse(rightAscending.begin(), rightAscending.end());

        split = 0;
        for (int j = 0; j < n; ++j) {
            if (leftArm[j] <= rightDescending[j])
                split = j + 1;
            else
                break;
        }

        windowStart = leftArm;
        complementStart = rightAscending;

        windowMiddle.reserve(n);
        complementMiddle.reserve(n);

        for (int j = split; j < n; ++j)
            windowMiddle.push_back(leftArm[j]);
        for (int j = 0; j < split; ++j)
            windowMiddle.push_back(rightDescending[j]);

        for (int j = 0; j < split; ++j)
            complementMiddle.push_back(leftArm[j]);
        for (int j = split; j < n; ++j)
            complementMiddle.push_back(rightDescending[j]);

        sort(windowMiddle.begin(), windowMiddle.end());
        sort(complementMiddle.begin(), complementMiddle.end());
    }

    bool feasible(const vector<int64>& red,
                  const vector<int64>& blue,
                  int64 limit) const {
        vector<Interval> redWindow =
            feasibleIntervals(red, limit, false);
        vector<Interval> redComplement =
            feasibleIntervals(red, limit, true);
        vector<Interval> blueWindow =
            feasibleIntervals(blue, limit, false);
        vector<Interval> blueComplement =
            feasibleIntervals(blue, limit, true);

        return intersects(redWindow, blueComplement) ||
               intersects(blueWindow, redComplement);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<int64> a(2 * n), red(n), blue(n);
    for (int64& x : a) cin >> x;
    for (int64& x : red) cin >> x;
    for (int64& x : blue) cin >> x;

    sort(red.begin(), red.end());
    sort(blue.begin(), blue.end());

    Solver solver(n, a);

    int64 low = -1;
    int64 high = 1000000000LL;

    while (high - low > 1) {
        int64 middle = low + (high - low) / 2;
        if (solver.feasible(red, blue, middle))
            high = middle;
        else
            low = middle;
    }

    cout << high << '\n';
    return 0;
}
