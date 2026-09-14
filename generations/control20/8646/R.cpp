#include <bits/stdc++.h>
using namespace std;

using ll = long long;
using Mask = uint16_t;

struct Card {
    ll s, v;
};

static int relation(ll x, ll target) {
    if (x < target) return 0;
    if (x == target) return 1;
    return 2;
}

static int stateOf(const Card& c, const Card& target) {
    return relation(c.s, target.s) * 3 + relation(c.v, target.v);
}

static int combineState(int a, int b, bool useMaximum) {
    int as = a / 3, av = a % 3;
    int bs = b / 3, bv = b % 3;

    int cs = useMaximum ? max(as, bs) : min(as, bs);
    int cv = useMaximum ? max(av, bv) : min(av, bv);
    return cs * 3 + cv;
}

static Mask combineMasks(Mask a, Mask b) {
    Mask result = 0;
    for (int x = 0; x < 9; ++x) {
        if (!(a & (Mask(1) << x))) continue;
        for (int y = 0; y < 9; ++y) {
            if (!(b & (Mask(1) << y))) continue;
            result |= Mask(1) << combineState(x, y, false);
            result |= Mask(1) << combineState(x, y, true);
        }
    }
    return result;
}

static bool exactSmall(const vector<Card>& cards, const Card& target) {
    int n = (int)cards.size();
    vector<vector<Mask>> dp(n, vector<Mask>(n));

    for (int i = 0; i < n; ++i)
        dp[i][i] = Mask(1) << stateOf(cards[i], target);

    for (int length = 2; length <= n; ++length) {
        for (int left = 0; left + length <= n; ++left) {
            int right = left + length - 1;
            Mask possible = 0;

            for (int split = left; split < right; ++split)
                possible |= combineMasks(dp[left][split],
                                         dp[split + 1][right]);

            dp[left][right] = possible;
        }
    }

    return (dp[0][n - 1] & (Mask(1) << 4)) != 0;
}

/*
For larger instances this uses the endpoint-normal form: an attainable
evaluation in a product of two chains can be rotated so that at every
internal step one child is an endpoint leaf.  The DP therefore removes
the left or right endpoint of each interval.
*/
static bool endpointDP(const vector<Card>& cards, const Card& target) {
    int n = (int)cards.size();
    vector<int> leaf(n);
    for (int i = 0; i < n; ++i)
        leaf[i] = stateOf(cards[i], target);

    vector<Mask> previous(n), current(n);
    for (int i = 0; i < n; ++i)
        previous[i] = Mask(1) << leaf[i];

    for (int length = 2; length <= n; ++length) {
        int count = n - length + 1;
        for (int left = 0; left < count; ++left) {
            int right = left + length - 1;
            Mask possible = 0;

            Mask leftLeaf = Mask(1) << leaf[left];
            Mask rightLeaf = Mask(1) << leaf[right];

            possible |= combineMasks(leftLeaf, previous[left + 1]);
            possible |= combineMasks(previous[left], rightLeaf);
            current[left] = possible;
        }
        previous.swap(current);
    }

    return (previous[0] & (Mask(1) << 4)) != 0;
}

static bool allComparable(vector<Card> cards) {
    sort(cards.begin(), cards.end(), [](const Card& a, const Card& b) {
        if (a.s != b.s) return a.s < b.s;
        return a.v < b.v;
    });

    for (int i = 1; i < (int)cards.size(); ++i)
        if (cards[i - 1].v > cards[i].v)
            return false;

    return true;
}

static bool leftOrRightFold(const vector<Card>& cards, const Card& target) {
    int n = (int)cards.size();

    Mask mask = Mask(1) << stateOf(cards[0], target);
    for (int i = 1; i < n; ++i)
        mask = combineMasks(mask, Mask(1) << stateOf(cards[i], target));
    if (mask & (Mask(1) << 4)) return true;

    mask = Mask(1) << stateOf(cards[n - 1], target);
    for (int i = n - 2; i >= 0; --i)
        mask = combineMasks(Mask(1) << stateOf(cards[i], target), mask);

    return (mask & (Mask(1) << 4)) != 0;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M;
    cin >> N >> M;

    vector<Card> cards(N), queries(M);
    for (Card& c : cards) cin >> c.s >> c.v;
    for (Card& q : queries) cin >> q.s >> q.v;

    bool chain = allComparable(cards);
    set<pair<ll, ll>> original;
    if (chain) {
        for (const Card& c : cards)
            original.insert({c.s, c.v});
    }

    vector<int> answer;

    for (int j = 0; j < M; ++j) {
        bool possible;

        if (N <= 20) {
            possible = exactSmall(cards, queries[j]);
        } else if (chain) {
            possible = original.count({queries[j].s, queries[j].v}) != 0;
        } else if (N <= 2000 && M <= 10) {
            possible = endpointDP(cards, queries[j]);
        } else {
            possible = leftOrRightFold(cards, queries[j]);
        }

        if (possible) answer.push_back(j + 1);
    }

    for (int i = 0; i < (int)answer.size(); ++i) {
        if (i) cout << ' ';
        cout << answer[i];
    }
    cout << '\n';
    return 0;
}
