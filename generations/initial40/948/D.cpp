#include <cstdio>
#include <vector>
#include <cstdint>

using int64 = long long;

constexpr int64 MOD = 1000000007LL;
constexpr int64 INV2 = 500000004LL;

class FastInput {
    static constexpr int BUFFER_SIZE = 1 << 16;
    char buffer[BUFFER_SIZE];
    int position = 0;
    int length = 0;

    char nextChar() {
        if (position >= length) {
            length = static_cast<int>(
                std::fread(buffer, 1, BUFFER_SIZE, stdin)
            );
            position = 0;
            if (length == 0) return '\0';
        }
        return buffer[position++];
    }

public:
    int64 nextLong() {
        char c;
        do {
            c = nextChar();
        } while (c != '\0' && c <= ' ');

        int64 value = 0;
        while (c >= '0' && c <= '9') {
            value = value * 10 + (c - '0');
            c = nextChar();
        }
        return value;
    }
};

inline int64 multiplyMod(int64 a, int64 b) {
    return (a % MOD) * (b % MOD) % MOD;
}

inline int64 triangularMod(int64 x) {
    return multiplyMod(multiplyMod(x, x + 1), INV2);
}

struct StackEntry {
    int64 height;
    int64 count;
    int64 value;
};

int main() {
    FastInput input;

    const int n = static_cast<int>(input.nextLong());
    std::vector<int64> heights(n);

    for (int i = 0; i < n; ++i) {
        heights[i] = input.nextLong();
    }

    std::vector<StackEntry> stack;
    stack.reserve(n);

    int64 currentSum = 0;
    int64 answer = 0;

    for (int i = 0; i < n; ++i) {
        const int64 width = input.nextLong();
        const int64 height = heights[i];
        const int64 value = triangularMod(height);

        int64 poppedCount = 0;
        int64 removedSum = 0;

        while (!stack.empty() && stack.back().height >= height) {
            const StackEntry entry = stack.back();
            stack.pop_back();

            poppedCount += entry.count;
            removedSum += multiplyMod(entry.value, entry.count);
            if (removedSum >= MOD) removedSum -= MOD;
        }

        int64 retainedSum = currentSum - removedSum;
        if (retainedSum < 0) retainedSum += MOD;

        // Sum over all right endpoints inside this section:
        // width * retainedSum
        // + value * sum_{k=1..width}(poppedCount + k).
        const int64 newIntervalCount =
            (multiplyMod(width, poppedCount) + triangularMod(width)) % MOD;

        int64 contribution =
            multiplyMod(width, retainedSum) +
            multiplyMod(value, newIntervalCount);
        if (contribution >= MOD) contribution -= MOD;

        answer += contribution;
        if (answer >= MOD) answer -= MOD;

        const int64 mergedCount = poppedCount + width;
        currentSum =
            retainedSum + multiplyMod(value, mergedCount);
        if (currentSum >= MOD) currentSum -= MOD;

        stack.push_back({height, mergedCount, value});
    }

    std::printf("%lld\n", answer);
    return 0;
}