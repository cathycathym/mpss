#include <bits/stdc++.h>
using namespace std;

static const long long MOD = 1000000007LL;

struct Node {
    long long prod;
    long long suf; // sum 2^(i-1) * product c[i..r]
    long long pre; // sum 2^(n-i) * product c[l..i]
};

int n;
vector<long long> pw2;
vector<Node> seg;

Node mergeNode(const Node& a, const Node& b) {
    Node res;
    res.prod = a.prod * b.prod % MOD;
    res.suf = (b.suf + b.prod * a.suf) % MOD;
    res.pre = (a.pre + a.prod * b.pre) % MOD;
    return res;
}

void build(int p, int l, int r) {
    if (l == r) {
        seg[p] = {0, 0, 0};
        return;
    }
    int m = (l + r) / 2;
    build(p * 2, l, m);
    build(p * 2 + 1, m + 1, r);
    seg[p] = mergeNode(seg[p * 2], seg[p * 2 + 1]);
}

void update(int p, int l, int r, int pos, int val) {
    if (l == r) {
        seg[p].prod = val;
        seg[p].suf = val * pw2[l - 1] % MOD;
        seg[p].pre = val * pw2[n - l] % MOD;
        return;
    }
    int m = (l + r) / 2;
    if (pos <= m) update(p * 2, l, m, pos, val);
    else update(p * 2 + 1, m + 1, r, pos, val);
    seg[p] = mergeNode(seg[p * 2], seg[p * 2 + 1]);
}

long long currentValue(long long sumC) {
    const Node& root = seg[1];
    long long ans = pw2[n - 1] * (sumC % MOD) % MOD;
    ans = (ans - root.suf - root.pre + (long long)n * root.prod) % MOD;
    if (ans < 0) ans += MOD;
    if (ans < 0) ans += MOD;
    return ans;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n;
    vector<long long> a(n + 1), b(n + 1);
    for (int i = 1; i <= n; ++i) cin >> a[i];
    for (int i = 1; i <= n; ++i) cin >> b[i];

    pw2.assign(n + 1, 1);
    for (int i = 1; i <= n; ++i) pw2[i] = pw2[i - 1] * 2 % MOD;

    vector<pair<long long, pair<int, int>>> events;
    for (int i = 1; i <= n; ++i) {
        long long lo = min(a[i], b[i]);
        long long hi = max(a[i], b[i]);
        events.push_back({lo + 1, {i, 1}});
        events.push_back({hi + 1, {i, 2}});
    }
    sort(events.begin(), events.end());

    seg.assign(4 * n + 5, {0, 0, 0});
    build(1, 1, n);

    long long answer = 0;
    long long sumC = 0;
    int at = 0;

    while (at < (int)events.size()) {
        long long level = events[at].first;

        while (at < (int)events.size() && events[at].first == level) {
            int pos = events[at].second.first;
            int val = events[at].second.second;
            if (val == 1) sumC += 1;
            else sumC += 1;
            update(1, 1, n, pos, val);
            ++at;
        }

        long long nextLevel = (at < (int)events.size() ? events[at].first : level);
        long long countLevels = nextLevel - level;
        answer = (answer + currentValue(sumC) * (countLevels % MOD)) % MOD;
    }

    cout << answer << '\n';
    return 0;
}
