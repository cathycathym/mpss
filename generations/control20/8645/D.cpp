#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

struct CutConstraints {
    int n;
    int leftBound, rightBound;
    bool impossible;
    vector<int> forbiddenDiff;

    explicit CutConstraints(int n)
        : n(n),
          leftBound(0),
          rightBound(n),
          impossible(false),
          forbiddenDiff(n + 2, 0) {}

    // Require min(cut, a) + min(n - cut, b) >= k.
    void requireAtLeast(int a, int b, int k) {
        if (impossible || k <= 0) return;

        if (k > min(n, a + b)) {
            impossible = true;
            return;
        }

        int lo = max(0, k - b);
        int hi = min(n, n + a - k);

        leftBound = max(leftBound, lo);
        rightBound = min(rightBound, hi);

        if (leftBound > rightBound) impossible = true;
    }

    // Forbid cuts satisfying min(cut, a) + min(n - cut, b) >= k.
    void forbidAtLeast(int a, int b, int k) {
        if (impossible) return;

        if (k <= 0) {
            ++forbiddenDiff[0];
            --forbiddenDiff[n + 1];
            return;
        }

        if (k > min(n, a + b)) return;

        int lo = max(0, k - b);
        int hi = min(n, n + a - k);

        if (lo <= hi) {
            ++forbiddenDiff[lo];
            --forbiddenDiff[hi + 1];
        }
    }

    vector<char> finish() const {
        vector<char> valid(n + 1, false);
        if (impossible) return valid;

        int forbiddenCount = 0;
        for (int cut = 0; cut <= n; ++cut) {
            forbiddenCount += forbiddenDiff[cut];
            if (leftBound <= cut && cut <= rightBound &&
                forbiddenCount == 0) {
                valid[cut] = true;
            }
        }
        return valid;
    }
};

vector<char> feasibleCuts(
    const vector<long long>& leftArm,
    const vector<long long>& rightArm,
    const vector<long long>& pots,
    long long distance,
    bool choosePrefixes
) {
    int n = static_cast<int>(pots.size());
    CutConstraints constraints(n);

    int leftCount = 0;
    int rightCount = 0;

    // The i-th selected value must be at most pots[i] + distance.
    for (int i = 0; i < n; ++i) {
        long long threshold = pots[i] + distance;

        while (leftCount < n && leftArm[leftCount] <= threshold)
            ++leftCount;
        while (rightCount < n && rightArm[rightCount] <= threshold)
            ++rightCount;

        int required = i + 1;

        if (choosePrefixes) {
            constraints.requireAtLeast(
                leftCount, rightCount, required
            );
        } else {
            int k = leftCount + rightCount - required + 1;
            constraints.forbidAtLeast(leftCount, rightCount, k);
        }
    }

    leftCount = 0;
    rightCount = 0;

    // The i-th selected value must be at least pots[i] - distance.
    for (int i = 0; i < n; ++i) {
        long long threshold = pots[i] - distance;

        while (leftCount < n && leftArm[leftCount] < threshold)
            ++leftCount;
        while (rightCount < n && rightArm[rightCount] < threshold)
            ++rightCount;

        int rank = i + 1;

        if (choosePrefixes) {
            constraints.forbidAtLeast(
                leftCount, rightCount, rank
            );
        } else {
            int k = leftCount + rightCount - rank + 1;
            constraints.requireAtLeast(leftCount, rightCount, k);
        }
    }

    return constraints.finish();
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<long long> seedlings(2 * n);
    vector<long long> red(n), blue(n);

    for (long long& value : seedlings) cin >> value;
    for (long long& value : red) cin >> value;
    for (long long& value : blue) cin >> value;

    sort(red.begin(), red.end());
    sort(blue.begin(), blue.end());

    vector<long long> leftArm(n), rightArm(n);
    for (int i = 0; i < n; ++i) {
        leftArm[i] = seedlings[i];
        rightArm[i] = seedlings[2 * n - 1 - i];
    }

    auto possible = [&](long long distance) {
        vector<char> redPrefixes =
            feasibleCuts(leftArm, rightArm, red, distance, true);
        vector<char> redSuffixes =
            feasibleCuts(leftArm, rightArm, red, distance, false);
        vector<char> bluePrefixes =
            feasibleCuts(leftArm, rightArm, blue, distance, true);
        vector<char> blueSuffixes =
            feasibleCuts(leftArm, rightArm, blue, distance, false);

        for (int cut = 0; cut <= n; ++cut) {
            if (redPrefixes[cut] && blueSuffixes[cut])
                return true;
            if (bluePrefixes[cut] && redSuffixes[cut])
                return true;
        }
        return false;
    };

    long long low = -1;
    long long high = 1000000000LL;

    while (high - low > 1) {
        long long middle = low + (high - low) / 2;
        if (possible(middle))
            high = middle;
        else
            low = middle;
    }

    cout << high << '\n';
    return 0;
}
