#include <iostream>
#include <vector>
using namespace std;

using int64 = long long;

const int64 MOD = 1000000007LL;
const int64 INV2 = 500000004LL;

int64 triangular(int64 x) {
    x %= MOD;
    return x * ((x + 1) % MOD) % MOD * INV2 % MOD;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    cin >> N;

    vector<int64> originalHeight(N);
    for (int i = 0; i < N; ++i) {
        cin >> originalHeight[i];
    }

    vector<int64> height;
    vector<int64> width;
    height.reserve(N);
    width.reserve(N);

    for (int i = 0; i < N; ++i) {
        int64 w;
        cin >> w;

        if (!height.empty() && height.back() == originalHeight[i]) {
            width.back() += w;
        } else {
            height.push_back(originalHeight[i]);
            width.push_back(w);
        }
    }

    int M = static_cast<int>(height.size());

    vector<int64> prefix(M + 1, 0);
    for (int i = 0; i < M; ++i) {
        prefix[i + 1] = prefix[i] + width[i];
    }

    vector<int64> leftDistance(M);
    vector<int> stack;
    stack.reserve(M);

    // Previous strictly smaller run.
    for (int i = 0; i < M; ++i) {
        while (!stack.empty() && height[stack.back()] >= height[i]) {
            stack.pop_back();
        }

        if (stack.empty()) {
            leftDistance[i] = prefix[i];
        } else {
            leftDistance[i] = prefix[i] - prefix[stack.back() + 1];
        }

        stack.push_back(i);
    }

    stack.clear();
    int64 answer = 0;
    const int64 totalWidth = prefix[M];

    // Next smaller-or-equal run.
    for (int i = M - 1; i >= 0; --i) {
        while (!stack.empty() && height[stack.back()] > height[i]) {
            stack.pop_back();
        }

        int64 rightDistance;
        if (stack.empty()) {
            rightDistance = totalWidth - prefix[i + 1];
        } else {
            rightDistance = prefix[stack.back()] - prefix[i + 1];
        }

        int64 w = width[i];
        int64 A = leftDistance[i];
        int64 B = rightDistance;

        int64 horizontal = 0;

        horizontal += ((w - 1) % MOD) * (A % MOD) % MOD;
        horizontal %= MOD;

        horizontal += triangular(w - 1);
        horizontal %= MOD;

        horizontal += ((A + w) % MOD) * ((B + 1) % MOD) % MOD;
        horizontal %= MOD;

        int64 vertical = triangular(height[i]);
        answer = (answer + horizontal * vertical) % MOD;

        stack.push_back(i);
    }

    cout << answer << '\n';
    return 0;
}