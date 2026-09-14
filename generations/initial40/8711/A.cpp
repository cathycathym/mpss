#include <bits/stdc++.h>
using namespace std;

using ll = long long;

struct DSU {
    vector<int> p, sz, xm, ym;
    int badY = 0;
    int cntX[2] = {0, 0};

    DSU() {
        p.push_back(0);
        sz.push_back(1);
        xm.push_back(0);
        ym.push_back(0);
    }

    int addNode() {
        int id = (int)p.size();
        p.push_back(id);
        sz.push_back(1);
        xm.push_back(0);
        ym.push_back(0);
        return id;
    }

    int find(int x) {
        if (p[x] == x) return x;
        return p[x] = find(p[x]);
    }

    void eraseContribution(int r) {
        if (ym[r] == 3) badY--;
        if (xm[r] & 1) cntX[0]--;
        if (xm[r] & 2) cntX[1]--;
    }

    void addContribution(int r) {
        if (ym[r] == 3) badY++;
        if (xm[r] & 1) cntX[0]++;
        if (xm[r] & 2) cntX[1]++;
    }

    void unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return;

        eraseContribution(a);
        eraseContribution(b);

        if (sz[a] < sz[b]) swap(a, b);
        p[b] = a;
        sz[a] += sz[b];
        xm[a] |= xm[b];
        ym[a] |= ym[b];

        addContribution(a);
    }

    void mark(int v, int xParity, int yParity1, int yParity2) {
        int r = find(v);
        eraseContribution(r);
        xm[r] |= (1 << xParity);
        ym[r] |= (1 << yParity1);
        ym[r] |= (1 << yParity2);
        addContribution(r);
    }

    bool validForParity(int parity) const {
        return badY == 0 && cntX[parity] > 0 && cntX[1 - parity] == 0;
    }
};

struct Edge {
    ll x, y1, y2;
    bool operator<(const Edge& other) const {
        return x < other.x;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    ll M;
    cin >> N >> M;

    vector<ll> x(N), y(N);
    for (int i = 0; i < N; i++) cin >> x[i] >> y[i];

    vector<Edge> events;
    for (int i = 0; i < N; i++) {
        int j = (i + 1) % N;
        if (x[i] == x[j] && y[i] != y[j]) {
            events.push_back({x[i], min(y[i], y[j]), max(y[i], y[j])});
        }
    }
    sort(events.begin(), events.end());

    DSU dsu;

    // Active vertical cross-section intervals:
    // left endpoint -> (right endpoint, component id)
    map<ll, pair<ll, int>> active;

    auto split = [&](ll pos) {
        auto it = active.upper_bound(pos);
        if (it == active.begin()) return;
        --it;
        ll l = it->first;
        ll r = it->second.first;
        int id = it->second.second;
        if (l < pos && pos < r) {
            it->second.first = pos;
            active[pos] = {r, id};
        }
    };

    auto mergeAdjacent = [&]() {
        for (auto it = active.begin(); it != active.end();) {
            auto nx = next(it);
            if (nx == active.end()) break;

            if (it->second.first == nx->first) {
                dsu.unite(it->second.second, nx->second.second);
                it->second.first = nx->second.first;
                active.erase(nx);
            } else {
                ++it;
            }
        }
    };

    ll answer = 0;
    ll previousX = 0;
    int i = 0;

    while (i < (int)events.size()) {
        ll curX = events[i].x;

        // The state before processing curX applies to all integer cuts
        // previousX < k <= curX.
        if (previousX + 1 <= curX) {
            for (int parity = 0; parity < 2; parity++) {
                if (!dsu.validForParity(parity)) continue;
                ll candidate = curX;
                if ((candidate & 1LL) != parity) candidate--;
                if (candidate >= previousX + 1) {
                    answer = max(answer, candidate);
                }
            }
        }

        int j = i;
        while (j < (int)events.size() && events[j].x == curX) j++;

        // Crossing a vertical polygon edge toggles the interior status
        // on its y-interval.
        for (int t = i; t < j; t++) {
            ll a = events[t].y1;
            ll b = events[t].y2;

            split(a);
            split(b);

            auto it = active.find(a);
            bool currentlyInside =
                (it != active.end() && it->second.first == b);

            if (currentlyInside) {
                // This piece of the current frontier disappears.
                dsu.mark(it->second.second, (int)(curX & 1LL),
                         (int)(a & 1LL), (int)(b & 1LL));
                active.erase(it);
            } else {
                // A new frontier component begins here.
                int id = dsu.addNode();
                dsu.mark(id, (int)(curX & 1LL),
                         (int)(a & 1LL), (int)(b & 1LL));
                active[a] = {b, id};
            }

            mergeAdjacent();
        }

        previousX = curX;
        i = j;
    }

    cout << answer << '\n';
    return 0;
}
