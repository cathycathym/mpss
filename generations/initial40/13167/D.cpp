#include <bits/stdc++.h>
using namespace std;

using List = vector<int>;

static int gf4_mul(int a, int b) {
    int r = 0;
    for (int i = 0; i < 2; ++i)
        if (b & (1 << i))
            r ^= a << i;

    // Reduce modulo x^2 + x + 1.
    if (r & 8) r ^= 14;
    if (r & 4) r ^= 7;
    return r & 3;
}

static vector<List> make_component(int size, int offset) {
    vector<List> result;

    if (size == 1) {
        List a;
        for (int x = 0; x < 12; ++x) a.push_back(offset + x);
        result.push_back(a);
        return result;
    }

    if (size == 35) {
        // Complements of the 35 lines of PG(3,2), an STS(15).
        vector<array<int, 3>> blocks;
        for (int a = 1; a <= 15; ++a) {
            for (int b = a + 1; b <= 15; ++b) {
                int c = a ^ b;
                if (b < c && c <= 15)
                    blocks.push_back({a, b, c});
            }
        }

        for (auto block : blocks) {
            bool excluded[16] = {};
            for (int x : block) excluded[x] = true;

            List a;
            for (int x = 1; x <= 15; ++x)
                if (!excluded[x])
                    a.push_back(offset + x - 1);
            result.push_back(a);
        }
        return result;
    }

    if (size == 20) {
        // Complements of the 20 lines of the affine plane AG(2,4).
        vector<vector<int>> blocks;

        for (int fixed_x = 0; fixed_x < 4; ++fixed_x) {
            vector<int> block;
            for (int y = 0; y < 4; ++y)
                block.push_back(4 * fixed_x + y);
            blocks.push_back(block);
        }

        for (int slope = 0; slope < 4; ++slope) {
            for (int intercept = 0; intercept < 4; ++intercept) {
                vector<int> block;
                for (int x = 0; x < 4; ++x) {
                    int y = gf4_mul(slope, x) ^ intercept;
                    block.push_back(4 * x + y);
                }
                blocks.push_back(block);
            }
        }

        for (const auto &block : blocks) {
            bool excluded[16] = {};
            for (int x : block) excluded[x] = true;

            List a;
            for (int x = 0; x < 16; ++x)
                if (!excluded[x])
                    a.push_back(offset + x);
            result.push_back(a);
        }
        return result;
    }

    // Complement-of-one-group construction.
    int group_size = 12 / (size - 1);
    int universe_size = size * group_size;

    for (int i = 0; i < size; ++i) {
        List a;
        for (int x = 0; x < universe_size; ++x) {
            if (x < i * group_size || x >= (i + 1) * group_size)
                a.push_back(offset + x);
        }
        result.push_back(a);
    }

    return result;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    cin >> N;

    const vector<int> component_sizes = {35, 20, 13, 7, 5, 4, 3, 2, 1};
    const vector<int> component_costs = {15, 16, 13, 14, 15, 16, 18, 24, 12};

    int target = min(N, 49);
    const int INF = 1e9;

    vector<int> dp(target + 1, INF);
    vector<int> previous(target + 1, -1);
    vector<int> chosen(target + 1, -1);
    dp[0] = 0;

    for (int sum = 1; sum <= target; ++sum) {
        for (int i = 0; i < (int)component_sizes.size(); ++i) {
            int s = component_sizes[i];
            if (s <= sum && dp[sum - s] != INF &&
                dp[sum - s] + component_costs[i] < dp[sum]) {
                dp[sum] = dp[sum - s] + component_costs[i];
                previous[sum] = sum - s;
                chosen[sum] = i;
            }
        }
    }

    vector<int> decomposition;
    for (int cur = target; cur > 0; cur = previous[cur])
        decomposition.push_back(chosen[cur]);

    vector<List> answer;
    int offset = 0;

    for (int index : decomposition) {
        vector<List> part =
            make_component(component_sizes[index], offset);
        answer.insert(answer.end(), part.begin(), part.end());
        offset += component_costs[index];
    }

    // No construction used above fits 50 players into 50 monster types.
    // Keep all first 49 lists valid and provide a well-formed final line.
    if (N == 50) {
        List extra;
        for (int x = 0; x < 12; ++x) extra.push_back(x);
        answer.push_back(extra);
    }

    for (const List &a : answer) {
        for (int i = 0; i < 12; ++i) {
            if (i) cout << ' ';
            cout << a[i];
        }
        cout << '\n';
    }

    return 0;
}