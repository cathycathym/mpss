#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

using int64 = long long;

struct Event {
    int64 x, y, z;
    int zIndex;
    int type; // 0 = student, 1 = query
    int id;
};

class FenwickTree {
private:
    int n;
    vector<int> tree;

public:
    explicit FenwickTree(int size) : n(size), tree(size + 1, 0) {}

    void add(int index, int value) {
        for (int i = index; i <= n; i += i & -i) {
            tree[i] += value;
        }
    }

    int prefixSum(int index) const {
        int result = 0;
        for (int i = index; i > 0; i -= i & -i) {
            result += tree[i];
        }
        return result;
    }
};

vector<Event> events;
vector<Event> bufferEvents;
vector<int64> answers;
FenwickTree* fenwick;

void cdq(int left, int right) {
    if (right - left <= 1) {
        return;
    }

    int middle = left + (right - left) / 2;

    cdq(left, middle);
    cdq(middle, right);

    // Both halves are sorted by y after the recursive calls.
    // Count student events in the left half that dominate queries
    // in the right half in the y and z dimensions.
    int leftPointer = left;

    for (int j = middle; j < right; ++j) {
        while (leftPointer < middle &&
               events[leftPointer].y <= events[j].y) {
            if (events[leftPointer].type == 0) {
                fenwick->add(events[leftPointer].zIndex, 1);
            }
            ++leftPointer;
        }

        if (events[j].type == 1) {
            answers[events[j].id] +=
                fenwick->prefixSum(events[j].zIndex);
        }
    }

    // Roll back all Fenwick tree changes made at this level.
    for (int i = left; i < leftPointer; ++i) {
        if (events[i].type == 0) {
            fenwick->add(events[i].zIndex, -1);
        }
    }

    // Merge the two halves by y.
    int i = left;
    int j = middle;
    int position = left;

    while (i < middle && j < right) {
        if (events[i].y <= events[j].y) {
            bufferEvents[position++] = events[i++];
        } else {
            bufferEvents[position++] = events[j++];
        }
    }

    while (i < middle) {
        bufferEvents[position++] = events[i++];
    }

    while (j < right) {
        bufferEvents[position++] = events[j++];
    }

    for (int k = left; k < right; ++k) {
        events[k] = bufferEvents[k];
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, Q;
    cin >> N >> Q;

    events.reserve(N + Q);
    vector<int64> zCoordinates;
    zCoordinates.reserve(N + Q);

    for (int i = 0; i < N; ++i) {
        int64 S, T;
        cin >> S >> T;

        Event event;
        event.x = -S;
        event.y = -T;
        event.z = -(S + T);
        event.zIndex = 0;
        event.type = 0;
        event.id = -1;

        events.push_back(event);
        zCoordinates.push_back(event.z);
    }

    answers.assign(Q, 0);

    for (int i = 0; i < Q; ++i) {
        int64 X, Y, Z;
        cin >> X >> Y >> Z;

        Event event;
        event.x = -X;
        event.y = -Y;
        event.z = -Z;
        event.zIndex = 0;
        event.type = 1;
        event.id = i;

        events.push_back(event);
        zCoordinates.push_back(event.z);
    }

    sort(zCoordinates.begin(), zCoordinates.end());
    zCoordinates.erase(
        unique(zCoordinates.begin(), zCoordinates.end()),
        zCoordinates.end()
    );

    for (Event& event : events) {
        event.zIndex = static_cast<int>(
            lower_bound(zCoordinates.begin(), zCoordinates.end(), event.z)
            - zCoordinates.begin()
        ) + 1;
    }

    // A student with the same x-coordinate as a query must come first,
    // because equality is allowed.
    sort(events.begin(), events.end(),
         [](const Event& a, const Event& b) {
             if (a.x != b.x) {
                 return a.x < b.x;
             }
             return a.type < b.type;
         });

    bufferEvents.resize(events.size());
    FenwickTree bit(static_cast<int>(zCoordinates.size()));
    fenwick = &bit;

    cdq(0, static_cast<int>(events.size()));

    for (int i = 0; i < Q; ++i) {
        cout << answers[i] << '\n';
    }

    return 0;
}