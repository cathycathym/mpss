#include <bits/stdc++.h>
using namespace std;

struct DSU {
    vector<int> p, sz, xp, yp, bad, active;
    long long badRoots = 0;
    long long activeRoots[2] = {0, 0};

    DSU(int n) : p(n), sz(n, 1), xp(n, -1), yp(n, -1),
                 bad(n, 0), active(n, 0) {
        iota(p.begin(), p.end(), 0);
    }

    int find(int a) {
        while (p[a] != a) {
            p[a] = p[p[a]];
            a = p[a];
        }
        return a;
    }

    void removeContribution(int r) {
        r = find(r);
        if (bad[r]) --badRoots;
        if (active[r] > 0 && xp[r] != -1) --activeRoots[xp[r]];
    }

    void addContribution(int r) {
        r = find(r);
        if (bad[r]) ++badRoots;
        if (active[r] > 0 && xp[r] != -1) ++activeRoots[xp[r]];
    }

    void addX(int a, int v) {
        a = find(a);
        removeContribution(a);
        if (xp[a] == -1) xp[a] = v;
        else if (xp[a] != v) bad[a] = 1;
        addContribution(a);
    }

    void addY(int a, int v) {
        a = find(a);
        removeContribution(a);
        if (yp[a] == -1) yp[a] = v;
        else if (yp[a] != v) bad[a] = 1;
        addContribution(a);
    }

    int unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return a;

        removeContribution(a);
        removeContribution(b);

        if (sz[a] < sz[b]) swap(a, b);
        p[b] = a;
        sz[a] += sz[b];

        if (xp[a] == -1) xp[a] = xp[b];
        else if (xp[b] != -1 && xp[a] != xp[b]) bad[a] = 1;

        if (yp[a] == -1) yp[a] = yp[b];
        else if (yp[b] != -1 && yp[a] != yp[b]) bad[a] = 1;

        bad[a] = bad[a] || bad[b];
        active[a] += active[b];

        addContribution(a);
        return a;
    }

    void changeActive(int a, int delta) {
        a = find(a);
        removeContribution(a);
        active[a] += delta;
        addContribution(a);
    }
};

struct Interval {
    long long r;
    int id;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    long long M;
    cin >> N >> M;

    vector<long long> x(N), y(N);
    for (int i = 0; i < N; ++i) cin >> x[i] >> y[i];

    map<long long, vector<pair<long long, long long>>> events;
    for (int i = 0; i < N; ++i) {
        int j = (i + 1) % N;
        if (x[i] == x[j]) {
            long long lo = min(y[i], y[j]);
            long long hi = max(y[i], y[j]);
            events[x[i]].push_back({lo, hi});
        }
    }

    DSU dsu(N + 5);
    int nextId = 0;

    // Active cross-section intervals immediately to the right of current x.
    map<long long, Interval> seg;

    auto eraseSegment = [&](map<long long, Interval>::iterator it) {
        dsu.changeActive(it->second.id, -1);
        seg.erase(it);
    };

    auto insertSegment = [&](long long l, long long r, int id) {
        if (l >= r) return;
        seg[l] = {r, id};
        dsu.changeActive(id, +1);
    };

    auto containing = [&](long long l, long long r) {
        auto it = seg.upper_bound(l);
        if (it == seg.begin()) return seg.end();
        --it;
        if (it->second.r >= r) return it;
        return seg.end();
    };

    auto coalesceAround = [&](long long start) {
        auto cur = seg.find(start);
        if (cur == seg.end()) return;

        if (cur != seg.begin()) {
            auto prv = prev(cur);
            if (prv->second.r == cur->first) {
                long long l = prv->first, r = cur->second.r;
                int a = prv->second.id, b = cur->second.id;
                eraseSegment(cur);
                eraseSegment(prv);
                int root = dsu.unite(a, b);
                insertSegment(l, r, root);
                cur = seg.find(l);
            }
        }

        auto nxt = next(cur);
        if (nxt != seg.end() && cur->second.r == nxt->first) {
            long long l = cur->first, r = nxt->second.r;
            int a = cur->second.id, b = nxt->second.id;
            eraseSegment(nxt);
            eraseSegment(cur);
            int root = dsu.unite(a, b);
            insertSegment(l, r, root);
        }
    };

    long long answer = 0;
    vector<long long> xs;
    xs.reserve(events.size());
    for (const auto &e : events) xs.push_back(e.first);

    for (int ei = 0; ei < (int)xs.size(); ++ei) {
        long long cx = xs[ei];

        for (auto edge : events[cx]) {
            long long lo = edge.first, hi = edge.second;
            auto it = containing(lo, hi);

            if (it != seg.end()) {
                // This vertical edge removes an already-active interval part.
                long long l = it->first, r = it->second.r;
                int id = it->second.id;

                dsu.addX(id, int(cx & 1));
                dsu.addY(id, int(lo & 1));
                dsu.addY(id, int(hi & 1));

                eraseSegment(it);
                insertSegment(l, lo, id);
                insertSegment(hi, r, id);
            } else {
                // This vertical edge adds a new interval part.
                int id = nextId++;
                dsu.addX(id, int(cx & 1));
                dsu.addY(id, int(lo & 1));
                dsu.addY(id, int(hi & 1));
                insertSegment(lo, hi, id);
                coalesceAround(lo);
            }
        }

        if (ei + 1 == (int)xs.size()) continue;
        long long nx = xs[ei + 1];

        // Cuts k in [cx+1, nx] see exactly the current active intervals.
        if (dsu.badRoots == 0 && !(dsu.activeRoots[0] && dsu.activeRoots[1])) {
            int neededParity = -1;
            if (dsu.activeRoots[0]) neededParity = 0;
            if (dsu.activeRoots[1]) neededParity = 1;

            long long candidate = nx;
            if (neededParity != -1 && (candidate & 1) != neededParity) --candidate;

            if (candidate >= cx + 1) answer = max(answer, candidate);
        }
    }

    cout << answer << '\n';
    return 0;
}
