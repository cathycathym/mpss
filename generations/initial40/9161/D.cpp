#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

struct Ship {
    int64 x, y;
    int dir;
};

struct Event {
    int64 time;
    int a, b;

    bool operator<(const Event& other) const {
        return time > other.time;
    }
};

const int DX[4] = {0, 0, 1, -1};
const int DY[4] = {-1, 1, 0, 0};

int directionId(char c) {
    if (c == 'N') return 0;
    if (c == 'S') return 1;
    if (c == 'E') return 2;
    return 3;
}

// 0: NS, 1: EW, 2: NE, 3: NW, 4: SE, 5: SW
int laneType(int a, int b) {
    if (a > b) swap(a, b);

    if (a == 0 && b == 1) return 0;
    if (a == 2 && b == 3) return 1;
    if (a == 0 && b == 2) return 2;
    if (a == 0 && b == 3) return 3;
    if (a == 1 && b == 2) return 4;
    return 5;
}

array<int, 3> involvedLanes(int dir) {
    array<int, 3> result;
    int at = 0;

    for (int other = 0; other < 4; ++other) {
        if (other != dir) {
            result[at++] = laneType(dir, other);
        }
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
        char direction;
        cin >> ships[i].x >> ships[i].y >> direction;
        ships[i].dir = directionId(direction);
    }

    auto laneKey = [&](int type, int id) -> int64 {
        const Ship& ship = ships[id];

        if (type == 0) return ship.x;
        if (type == 1) return ship.y;
        if (type == 2 || type == 5) return ship.x - ship.y;
        return ship.x + ship.y;
    };

    auto lanePosition = [&](int type, int id) -> int64 {
        if (type == 0) return ships[id].y;
        return ships[id].x;
    };

    using OrderedLane = set<pair<int64, int>>;
    vector<map<int64, OrderedLane>> lanes(6);

    for (int id = 0; id < n; ++id) {
        for (int type : involvedLanes(ships[id].dir)) {
            lanes[type][laneKey(type, id)].insert(
                {lanePosition(type, id), id}
            );
        }
    }

    vector<char> alive(n, true);

    vector<map<pair<int64, int64>, int>> startingPosition(4);
    for (int id = 0; id < n; ++id) {
        startingPosition[ships[id].dir][{ships[id].x, ships[id].y}] = id;
    }

    auto collisionTime = [&](int a, int b) -> int64 {
        int64 time = -1;

        int64 coefficientX = DX[ships[a].dir] - DX[ships[b].dir];
        int64 differenceX = ships[b].x - ships[a].x;

        if (coefficientX == 0) {
            if (differenceX != 0) return -1;
        } else {
            if (differenceX % coefficientX != 0) return -1;
            time = differenceX / coefficientX;
        }

        int64 coefficientY = DY[ships[a].dir] - DY[ships[b].dir];
        int64 differenceY = ships[b].y - ships[a].y;

        if (coefficientY == 0) {
            if (differenceY != 0) return -1;
        } else {
            if (differenceY % coefficientY != 0) return -1;

            int64 otherTime = differenceY / coefficientY;
            if (time != -1 && time != otherTime) return -1;
            time = otherTime;
        }

        return time;
    };

    priority_queue<Event> events;
    int64 currentTime = 0;

    auto addCandidate = [&](int a, int b) {
        if (a == -1 || b == -1 || !alive[a] || !alive[b]) return;
        if (ships[a].dir == ships[b].dir) return;

        int64 time = collisionTime(a, b);
        if (time > currentTime) {
            events.push({time, a, b});
        }
    };

    for (int type = 0; type < 6; ++type) {
        for (auto& entry : lanes[type]) {
            const auto& lane = entry.second;
            if (lane.size() < 2) continue;

            auto left = lane.begin();
            auto right = next(left);

            while (right != lane.end()) {
                addCandidate(left->second, right->second);
                ++left;
                ++right;
            }
        }
    }

    auto adjacent = [&](int a, int b) -> bool {
        if (!alive[a] || !alive[b]) return false;
        if (ships[a].dir == ships[b].dir) return false;

        int type = laneType(ships[a].dir, ships[b].dir);
        int64 key = laneKey(type, a);

        auto laneIt = lanes[type].find(key);
        if (laneIt == lanes[type].end()) return false;

        const auto& lane = laneIt->second;
        auto it = lane.find({lanePosition(type, a), a});
        if (it == lane.end()) return false;

        if (it != lane.begin() && prev(it)->second == b) return true;

        auto nextIt = next(it);
        return nextIt != lane.end() && nextIt->second == b;
    };

    auto eraseShip = [&](int id) {
        if (!alive[id]) return;
        alive[id] = false;

        startingPosition[ships[id].dir].erase({ships[id].x, ships[id].y});

        for (int type : involvedLanes(ships[id].dir)) {
            int64 key = laneKey(type, id);
            auto laneIt = lanes[type].find(key);
            if (laneIt == lanes[type].end()) continue;

            auto& lane = laneIt->second;
            auto it = lane.find({lanePosition(type, id), id});
            if (it == lane.end()) continue;

            int leftId = -1;
            int rightId = -1;

            if (it != lane.begin()) leftId = prev(it)->second;
            auto rightIt = next(it);
            if (rightIt != lane.end()) rightId = rightIt->second;

            lane.erase(it);
            if (lane.empty()) {
                lanes[type].erase(laneIt);
            }

            addCandidate(leftId, rightId);
        }
    };

    while (!events.empty()) {
        int64 time = events.top().time;
        vector<Event> simultaneous;

        while (!events.empty() && events.top().time == time) {
            simultaneous.push_back(events.top());
            events.pop();
        }

        if (time <= currentTime) continue;

        set<pair<int64, int64>> crashPositions;

        for (const Event& event : simultaneous) {
            if (!alive[event.a] || !alive[event.b]) continue;
            if (!adjacent(event.a, event.b)) continue;
            if (collisionTime(event.a, event.b) != time) continue;

            int64 crashX =
                ships[event.a].x + int64(DX[ships[event.a].dir]) * time;
            int64 crashY =
                ships[event.a].y + int64(DY[ships[event.a].dir]) * time;

            crashPositions.insert({crashX, crashY});
        }

        if (crashPositions.empty()) continue;

        vector<int> toErase;
        vector<char> marked(n, false);

        for (const auto& position : crashPositions) {
            int64 crashX = position.first;
            int64 crashY = position.second;

            vector<int> present;

            for (int dir = 0; dir < 4; ++dir) {
                int64 startX = crashX - int64(DX[dir]) * time;
                int64 startY = crashY - int64(DY[dir]) * time;

                auto it = startingPosition[dir].find({startX, startY});
                if (it != startingPosition[dir].end() && alive[it->second]) {
                    present.push_back(it->second);
                }
            }

            if (present.size() >= 2) {
                for (int id : present) {
                    if (!marked[id]) {
                        marked[id] = true;
                        toErase.push_back(id);
                    }
                }
            }
        }

        currentTime = time;

        for (int id : toErase) {
            eraseShip(id);
        }
    }

    for (int id = 0; id < n; ++id) {
        if (alive[id]) {
            cout << id + 1 << '\n';
        }
    }

    return 0;
}