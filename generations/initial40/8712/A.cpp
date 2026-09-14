#include <bits/stdc++.h>
using namespace std;

static const long long MOD = 1000000007LL;

struct Node {
    long long prod, constantPart, suffixPart, prefixPart, bothPart;
};

Node mergeNode(const Node& a, const Node& b) {
    return {
        a.prod * b.prod % MOD,
        (a.constantPart + b.constantPart) % MOD,
        (a.suffixPart * b.prod + b.suffixPart) % MOD,
        (a.prefixPart + a.prod * b.prefixPart) % MOD,
        (a.bothPart * b.prod + a.prod * b.bothPart) % MOD
    };
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<long long> a(n + 1), b(n + 1);
    for (int i = 1; i <= n; ++i) cin >> a[i];
    for (int i = 1; i <= n; ++i) cin >> b[i];

    vector<long long> pow2(n + 1, 1);
    for (int i = 1; i <= n; ++i) pow2[i] = pow2[i - 1] * 2 % MOD;

    int base = 1;
    while (base < n) base <<= 1;

    vector<Node> seg(2 * base, {1, 0, 0, 0, 0});
    vector<int> active(n + 1, 0);
    vector<long long> d(n + 1, 2);

    auto makeLeaf = [&](int pos) -> Node {
        if (pos > n) return {1, 0, 0, 0, 0};

        if (!active[pos]) return {d[pos], 0, 0, 0, 0};

        // While active, exactly one of the two choices is below this water level.
        return {
            1,
            pow2[n - 1],
            pow2[pos - 1],
            pow2[n - pos],
            1
        };
    };

    for (int i = 1; i <= n; ++i) seg[base + i - 1] = makeLeaf(i);
    for (int i = base - 1; i >= 1; --i)
        seg[i] = mergeNode(seg[i << 1], seg[i << 1 | 1]);

    auto update = [&](int pos) {
        int p = base + pos - 1;
        seg[p] = makeLeaf(pos);
        for (p >>= 1; p; p >>= 1)
            seg[p] = mergeNode(seg[p << 1], seg[p << 1 | 1]);
    };

    // type 1: at the larger height, this segment becomes active (d changes 2 -> 1)
    // type 0: at the smaller height, this segment stops being active (d changes 1 -> 0)
    vector<tuple<long long, int, int>> events;
    events.reserve(2 * n);

    for (int i = 1; i <= n; ++i) {
        long long low = min(a[i], b[i]);
        long long high = max(a[i], b[i]);
        events.emplace_back(high, i, 1);
        events.emplace_back(low, i, 0);
    }

    sort(events.begin(), events.end(), greater<tuple<long long, int, int>>());

    long long answer = 0;
    int ptr = 0;

    while (ptr < (int)events.size()) {
        long long height = get<0>(events[ptr]);

        while (ptr < (int)events.size() && get<0>(events[ptr]) == height) {
            int pos = get<1>(events[ptr]);
            int type = get<2>(events[ptr]);

            if (type == 1) {
                d[pos] = 1;
                active[pos] = 1;
            } else {
                d[pos] = 0;
                active[pos] = 0;
            }
            update(pos);
            ++ptr;
        }

        long long nextHeight = (ptr < (int)events.size() ? get<0>(events[ptr]) : 0);
        long long levels = height - nextHeight;

        long long contribution =
            (seg[1].constantPart - seg[1].suffixPart - seg[1].prefixPart + seg[1].bothPart) % MOD;
        if (contribution < 0) contribution += MOD;
        if (contribution < 0) contribution += MOD;

        answer = (answer + contribution * (levels % MOD)) % MOD;
    }

    cout << answer << '\n';
    return 0;
}
