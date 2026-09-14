#include <bits/stdc++.h>
using namespace std;

static constexpr long long MOD = 1'000'000'007LL;

struct Node {
    long long product;
    long long suffixSum;
    long long prefixSum;
};

int n;
vector<long long> powerOfTwo;
vector<Node> tree;

Node mergeNodes(const Node& left, const Node& right) {
    return {
        left.product * right.product % MOD,
        (right.suffixSum + right.product * left.suffixSum) % MOD,
        (left.prefixSum + left.product * right.prefixSum) % MOD
    };
}

void build(int node, int left, int right) {
    if (left == right) {
        tree[node] = {0, 0, 0};
        return;
    }

    int middle = (left + right) / 2;
    build(node * 2, left, middle);
    build(node * 2 + 1, middle + 1, right);
    tree[node] = mergeNodes(tree[node * 2], tree[node * 2 + 1]);
}

void update(int node, int left, int right, int position, int value) {
    if (left == right) {
        tree[node] = {
            value,
            value * powerOfTwo[left - 1] % MOD,
            value * powerOfTwo[n - left] % MOD
        };
        return;
    }

    int middle = (left + right) / 2;
    if (position <= middle) {
        update(node * 2, left, middle, position, value);
    } else {
        update(node * 2 + 1, middle + 1, right, position, value);
    }

    tree[node] = mergeNodes(tree[node * 2], tree[node * 2 + 1]);
}

long long trappedAssignments(long long sumChoicesBelow) {
    const Node& root = tree[1];

    long long result =
        powerOfTwo[n - 1] * (sumChoicesBelow % MOD) % MOD;

    result = (result - root.suffixSum) % MOD;
    result = (result - root.prefixSum) % MOD;
    result = (result + (n % MOD) * root.product) % MOD;

    if (result < 0) result += MOD;
    return result;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n;

    vector<long long> a(n + 1), b(n + 1);
    for (int i = 1; i <= n; ++i) cin >> a[i];
    for (int i = 1; i <= n; ++i) cin >> b[i];

    powerOfTwo.assign(n + 1, 1);
    for (int i = 1; i <= n; ++i) {
        powerOfTwo[i] = powerOfTwo[i - 1] * 2 % MOD;
    }

    struct Event {
        long long level;
        int position;
        int choicesBelow;
    };

    vector<Event> events;
    events.reserve(2 * n);

    for (int i = 1; i <= n; ++i) {
        long long low = min(a[i], b[i]);
        long long high = max(a[i], b[i]);

        events.push_back({low + 1, i, 1});
        events.push_back({high + 1, i, 2});
    }

    sort(events.begin(), events.end(), [](const Event& x, const Event& y) {
        return x.level < y.level;
    });

    tree.assign(4 * n + 5, {0, 0, 0});
    build(1, 1, n);

    long long answer = 0;
    long long sumChoicesBelow = 0;

    for (int index = 0; index < (int)events.size();) {
        long long level = events[index].level;

        while (index < (int)events.size() && events[index].level == level) {
            int position = events[index].position;
            int newValue = events[index].choicesBelow;

            sumChoicesBelow += 1;
            update(1, 1, n, position, newValue);
            ++index;
        }

        if (index < (int)events.size()) {
            long long nextLevel = events[index].level;
            long long numberOfLevels = nextLevel - level;

            answer = (
                answer +
                trappedAssignments(sumChoicesBelow) *
                    (numberOfLevels % MOD)
            ) % MOD;
        }
    }

    cout << answer << '\n';
    return 0;
}