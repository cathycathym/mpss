#include <bits/stdc++.h>
using namespace std;

using ll = long long;

struct Node {
    int ship, channel;
    ll q;
    int vel;
    int prv = -1, nxt = -1;
};

struct Event {
    ll t;
    int a, b;
    bool operator<(const Event& other) const {
        return t > other.t;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<ll> x(n), y(n);
    vector<char> d(n);
    for (int i = 0; i < n; ++i) cin >> x[i] >> y[i] >> d[i];

    vector<Node> nodes;
    vector<vector<int>> channelNodes;
    vector<vector<int>> memberships(n);

    // key: (collision-family, invariant coordinate)
    map<pair<int, ll>, int> channelId;

    auto addMembership = [&](int ship, int type, ll key, ll q, int vel) {
        pair<int, ll> mapKey = {type, key};
        int cid;
        auto it = channelId.find(mapKey);
        if (it == channelId.end()) {
            cid = (int)channelNodes.size();
            channelId[mapKey] = cid;
            channelNodes.push_back({});
        } else {
            cid = it->second;
        }

        int id = (int)nodes.size();
        nodes.push_back({ship, cid, q, vel});
        channelNodes[cid].push_back(id);
        memberships[ship].push_back(id);
    };

    for (int i = 0; i < n; ++i) {
        ll u = x[i] + y[i];
        ll v = x[i] - y[i];

        if (d[i] == 'N') {
            // N-S: fixed x, y decreases
            addMembership(i, 0, x[i], y[i], -1);
            // E-W? N-W: fixed u, N increases in v
            addMembership(i, 3, u, v, +1);
            // E-N: fixed v, N decreases in u
            addMembership(i, 4, v, u, -1);
        } else if (d[i] == 'S') {
            // N-S: fixed x, y increases
            addMembership(i, 0, x[i], y[i], +1);
            // E-S: fixed u, S decreases in v
            addMembership(i, 2, u, v, -1);
            // W-S: fixed v, S increases in u
            addMembership(i, 5, v, u, +1);
        } else if (d[i] == 'E') {
            // E-W: fixed y, x increases
            addMembership(i, 1, y[i], x[i], +1);
            // E-S: fixed u, E increases in v
            addMembership(i, 2, u, v, +1);
            // E-N: fixed v, E increases in u
            addMembership(i, 4, v, u, +1);
        } else { // W
            // E-W: fixed y, x decreases
            addMembership(i, 1, y[i], x[i], -1);
            // W-N: fixed u, W decreases in v
            addMembership(i, 3, u, v, -1);
            // W-S: fixed v, W decreases in u
            addMembership(i, 5, v, u, -1);
        }
    }

    for (auto& vec : channelNodes) {
        sort(vec.begin(), vec.end(), [&](int a, int b) {
            return nodes[a].q < nodes[b].q;
        });
        for (int i = 0; i < (int)vec.size(); ++i) {
            if (i) nodes[vec[i]].prv = vec[i - 1];
            if (i + 1 < (int)vec.size()) nodes[vec[i]].nxt = vec[i + 1];
        }
    }

    priority_queue<Event> pq;
    vector<char> alive(n, true);

    auto schedule = [&](int a, int b) {
        if (a == -1 || b == -1) return;
        if (nodes[a].vel != +1 || nodes[b].vel != -1) return;
        ll delta = nodes[b].q - nodes[a].q;
        if (delta <= 0) return;
        pq.push({delta / 2, a, b});
    };

    for (const auto& vec : channelNodes) {
        for (int id : vec) {
            schedule(id, nodes[id].nxt);
        }
    }

    auto positionAt = [&](int ship, ll t) {
        ll px = x[ship], py = y[ship];
        if (d[ship] == 'N') py -= t;
        if (d[ship] == 'S') py += t;
        if (d[ship] == 'E') px += t;
        if (d[ship] == 'W') px -= t;
        return make_pair(px, py);
    };

    while (!pq.empty()) {
        ll now = pq.top().t;

        // Process all currently known collisions at this exact time.
        // Removing them may expose further collisions also happening now,
        // so repeat until no event with this timestamp remains.
        while (!pq.empty() && pq.top().t == now) {
            map<pair<ll, ll>, vector<int>> crashes;
            vector<Event> valid;

            while (!pq.empty() && pq.top().t == now) {
                Event e = pq.top();
                pq.pop();

                if (!alive[nodes[e.a].ship] || !alive[nodes[e.b].ship]) continue;
                if (nodes[e.a].nxt != e.b || nodes[e.b].prv != e.a) continue;

                valid.push_back(e);
                auto pos = positionAt(nodes[e.a].ship, now);
                crashes[pos].push_back(nodes[e.a].ship);
                crashes[pos].push_back(nodes[e.b].ship);
            }

            if (valid.empty()) break;

            vector<char> removeShip(n, false);
            for (auto& entry : crashes) {
                for (int ship : entry.second) removeShip[ship] = true;
            }

            vector<int> toRemove;
            for (int i = 0; i < n; ++i) {
                if (removeShip[i] && alive[i]) {
                    alive[i] = false;
                    toRemove.push_back(i);
                }
            }

            for (int ship : toRemove) {
                for (int id : memberships[ship]) {
                    int l = nodes[id].prv;
                    int r = nodes[id].nxt;

                    if (l != -1) nodes[l].nxt = r;
                    if (r != -1) nodes[r].prv = l;

                    schedule(l, r);
                }
            }
        }
    }

    for (int i = 0; i < n; ++i) {
        if (alive[i]) cout << i + 1 << '\n';
    }

    return 0;
}
