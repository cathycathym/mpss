#include <cstdio>
#include <vector>
#include <cstdint>

using int64 = long long;

static const int64 MOD = 1000000007LL;
static const int64 INV2 = 500000004LL;

class FastInput {
    static const int BUFFER_SIZE = 1 << 16;
    char buffer[BUFFER_SIZE];
    int position = 0;
    int length = 0;

    char nextChar() {
        if (position == length) {
            length = static_cast<int>(
                std::fread(buffer, 1, BUFFER_SIZE, stdin)
            );
            position = 0;
            if (length == 0) return 0;
        }
        return buffer[position++];
    }

public:
    int64 nextLong() {
        char c;
        do {
            c = nextChar();
        } while (c <= ' ' && c != 0);

        int64 value = 0;
        while (c >= '0' && c <= '9') {
            value = value * 10 + (c - '0');
            c = nextChar();
        }
        return value;
    }
};

int64 multiplyMod(int64 a, int64 b) {
    return (a % MOD) * (b % MOD) % MOD;
}

int64 triangularMod(int64 x) {
    return multiplyMod(multiplyMod(x, x + 1), INV2);
}

struct StackEntry {
    int64 height;
    int64 count;
    int64 value;
};

int main() {
    FastInput input;

    int n = static_cast<int>(input.nextLong());
    std::vector<int64> heights(n);

    for (int i = 0; i < n; ++i) {
        heights[i] = input.nextLong();
    }

    std::vector<StackEntry> stack;
    stack.reserve(n);

    int64 current = 0;
    int64 answer = 0;

    for (int i = 0; i < n; ++i) {
        int64 width = input.nextLong();
        int64 height = heights[i];
        int64 value = triangularMod(height);

        int64 poppedCount = 0;
        int64 removedContribution = 0;

        while (!stack.empty() && stack.back().height >= height) {
            const StackEntry &entry = stack.back();

            poppedCount += entry.count;
            removedContribution +=
                multiplyMod(entry.value, entry.count);
            if (removedContribution >= MOD) {
                removedContribution -= MOD;
            }

            stack.pop_back();
        }

        int64 retained = current - removedContribution;
        if (retained < 0) retained += MOD;

        int64 widthMod = width % MOD;
        int64 initialCountMod = (poppedCount + 1) % MOD;

        int64 countSum =
            multiplyMod(widthMod, initialCountMod);
        countSum += triangularMod(width - 1);
        if (countSum >= MOD) countSum -= MOD;

        int64 blockContribution =
            multiplyMod(widthMod, retained);
        blockContribution += multiplyMod(value, countSum);
        if (blockContribution >= MOD) blockContribution -= MOD;

        answer += blockContribution;
        if (answer >= MOD) answer -= MOD;

        int64 finalCount = poppedCount + width;
        current = retained + multiplyMod(value, finalCount);
        if (current >= MOD) current -= MOD;

        stack.push_back({height, finalCount, value});
    }

    std::printf("%lld\n", answer);
    return 0;
}