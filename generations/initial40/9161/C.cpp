#include <bits/stdc++.h>
using namespace std;

using ll = long long;

struct Ship {
    ll x, y;
    int d;
};

struct Event {
    ll t;
    int a, b;
    bool operator<(const Event& other) const {
        return t > other.t;
    }
};

int dirId(char c) {
    if (c == 'N') return 0;
    if (c == 'S') return 1;
    if (c == 'E') return 2;
    return 3;
}

int vx[4] = {0, 0, 1, -1};
int vy[4] = {-1, 1, 0, 0};

// Lane types:
// 0 NS, 1 EW, 2 NE, 3 NW, 4 SE, 5 SW
int laneType(int a, int b) {
    if (a > b) swap(a, b);
    if (a == 0 && b == 1) return 0;
    if (a == 2 && b == 3) return 1;
    if (a == 0 && b == 2) return 2;
    if (a == 0 && b == 3) return 3;
    if (a == 1 && b == 2) return 4;
    return 5;
}

vector<int> involvedTypes(int d) {
    vector<int> result;
    for (int other = 0; other < 4; ++other) {
        if (other != d) result.push_back(laneType(d, other));
    }
    return result;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<Ship> ships(n);
    for (int i = 0; i < n; ++i) {
        char c;
        cin >> ships[i].x >> ships[i].y >> c;
        ships[i].d = dirId(c);
    }

    auto laneKey = [&](int type, int id) -> ll {
        const Ship& s = ships[id];
        if (type == 0) return s.x;
        if (type == 1) return s.y;
        if (type == 2 || type == 5) return s.x - s.y;
        return s.x + s.y;
    };

    auto lanePosition = [&](int type, int id) -> ll {
        const Ship& s = ships[id];
        if (type == 0) return s.y;
        return s.x;
    };

    // lanes[type][invariant] = ordered ships by the remaining coordinate
    vector<map<ll, set<pair<ll, int>>>> lanes(6);

    for (int i = 0; i < n; ++i) {
        for (int type : involvedTypes(ships[i].d)) {
            lanes[type][laneKey(type, i)].insert({lanePosition(type, i), i});
        }
    }

    vector<bool> alive(n, true);

    // Live starting positions, split by direction.
    vector<map<pair<ll, ll>, int>> positionMap(4);
    for (int i = 0; i < n; ++i) {
        positionMap[ships[i].d][{ships[i].x, ships[i].y}] = i;
    }

    auto collisionTime = [&](int a, int b) -> ll {
        ll t = -1;

        ll coefX = vx[ships[a].d] - vx[ships[b].d];
        ll needX = ships[b].x - ships[a].x;

        if (coefX != 0) {
            if (needX % coefX != 0) return -1;
            t = needX / coefX;
        } else if (needX != 0) {
            return -1;
        }

        ll coefY = vy[ships[a].d] - vy[ships[b].d];
        ll needY = ships[b].y - ships[a].y;

        if (coefY != 0) {
            if (needY % coefY != 0) return -1;
            ll ty = needY / coefY;
            if (t != -1 && t != ty) return -1;
            t = ty;
        } else if (needY != 0) {
            return -1;
        }

        return t;
    };

    priority_queue<Event> pq;
    ll currentTime = 0;

    auto addCandidate = [&](int a, int b) {
        if (a < 0 || b < 0 || !alive[a] || !alive[b]) return;
        if (ships[a].d == ships[b].d) return;

        ll t = collisionTime(a, b);
        if (t > currentTime) {
            pq.push({t, a, b});
        }
    };

    // Initially, every neighboring mixed-direction pair is a candidate.
    for (int type = 0; type < 6; ++type) {
        for (auto& outer : lanes[type]) {
            auto& st = outer.second;
            if (st.empty()) continue;
            auto it = st.begin();
            auto nextIt = it;
            ++nextIt;
            while (nextIt != st.end()) {
                addCandidate(it->second, nextIt->second);
                ++it;
                ++nextIt;
            }
        }
    }

    auto areAdjacent = [&](int a, int b) -> bool {
        if (!alive[a] || !alive[b] || ships[a].d == ships[b].d) return false;

        int type = laneType(ships[a].d, ships[b].d);
        ll key = laneKey(type, a);

        auto outer = lanes[type].find(key);
        if (outer == lanes[type].end()) return false;

        auto& st = outer->second;
        auto it = st.find({lanePosition(type, a), a});
        if (it == st.end()) return false;

        if (it != st.begin()) {
            auto prevIt = prev(it);
            if (prevIt->second == b) return true;
        }

        auto nextIt = next(it);
        return nextIt != st.end() && nextIt->second == b;
    };

    auto eraseShip = [&](int id) {
        if (!alive[id]) return;

        alive[id] = false;
        positionMap[ships[id].d].erase({ships[id].x, ships[id].y});

        for (int type : involvedTypes(ships[id].d)) {
            ll key = laneKey(type, id);
            auto outer = lanes[type].find(key);
            if (outer == lanes[type].end()) continue;

            auto& st = outer->second;
            auto it = st.find({lanePosition(type, id), id});
            if (it == st.end()) continue;

            int leftId = -1, rightId = -1;
            if (it != st.begin()) leftId = prev(it)->second;
            auto nextIt = next(it);
            if (nextIt != st.end()) rightId = nextIt->second;

            st.erase(it);
            if (st.empty()) lanes[type].erase(outer);

            if (leftId != -1 && rightId != -1) {
                addCandidate(leftId, rightId);
            }
        }
    };

    while (!pq.empty()) {
        Event event = pq.top();
        pq.pop();

        if (!alive[event.a] || !alive[event.b]) continue;
        if (!areAdjacent(event.a, event.b)) continue;

        ll t = collisionTime(event.a, event.b);
        if (t != event.t || t <= currentTime) continue;

        currentTime = t;

        ll px = ships[event.a].x + vx[ships[event.a].d] * t;
        ll py = ships[event.a].y + vy[ships[event.a].d] * t;

        vector<int> sunk;
        for (int d = 0; d < 4; ++d) {
            ll startX = px - vx[d] * t;
            ll startY = py - vy[d] * t;

            auto it = positionMap[d].find({startX, startY});
            if (it != positionMap[d].end() && alive[it->second]) {
                sunk.push_back(it->second);
            }
        }

        if (sunk.size() >= 2) {
            for (int id : sunk) eraseShip(id);
        }
    }

    for (int i = 0; i < n; ++i) {
        if (alive[i]) {
            cout << i + 1 << '\n';
        }
    }

    return 0;
}
