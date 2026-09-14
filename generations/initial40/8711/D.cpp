#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

struct DSU {
    vector<int> parent, size, xParity, yParity, bad, active;
    int badComponents = 0;
    int activeByParity[2] = {0, 0};

    explicit DSU(int n)
        : parent(n), size(n, 1), xParity(n, -1), yParity(n, -1),
          bad(n, 0), active(n, 0) {
        iota(parent.begin(), parent.end(), 0);
    }

    int find(int v) {
        if (parent[v] == v) return v;
        return parent[v] = find(parent[v]);
    }

    void removeContribution(int r) {
        r = find(r);
        if (bad[r]) --badComponents;
        if (active[r] > 0 && xParity[r] != -1) {
            --activeByParity[xParity[r]];
        }
    }

    void addContribution(int r) {
        r = find(r);
        if (bad[r]) ++badComponents;
        if (active[r] > 0 && xParity[r] != -1) {
            ++activeByParity[xParity[r]];
        }
    }

    void addXConstraint(int v, int parity) {
        v = find(v);
        removeContribution(v);

        if (xParity[v] == -1) xParity[v] = parity;
        else if (xParity[v] != parity) bad[v] = 1;

        addContribution(v);
    }

    void addYConstraint(int v, int parity) {
        v = find(v);
        removeContribution(v);

        if (yParity[v] == -1) yParity[v] = parity;
        else if (yParity[v] != parity) bad[v] = 1;

        addContribution(v);
    }

    int unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return a;

        removeContribution(a);
        removeContribution(b);

        if (size[a] < size[b]) swap(a, b);
        parent[b] = a;
        size[a] += size[b];

        if (xParity[a] == -1) {
            xParity[a] = xParity[b];
        } else if (xParity[b] != -1 && xParity[a] != xParity[b]) {
            bad[a] = 1;
        }

        if (yParity[a] == -1) {
            yParity[a] = yParity[b];
        } else if (yParity[b] != -1 && yParity[a] != yParity[b]) {
            bad[a] = 1;
        }

        bad[a] |= bad[b];
        active[a] += active[b];

        addContribution(a);
        return a;
    }

    void changeActive(int v, int delta) {
        v = find(v);
        removeContribution(v);
        active[v] += delta;
        addContribution(v);
    }
};

struct Segment {
    int64 right;
    int component;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    int64 M;
    cin >> N >> M;

    vector<pair<int64, int64>> original(N);
    for (auto &[x, y] : original) cin >> x >> y;

    // Remove redundant vertices lying inside a straight boundary segment.
    vector<pair<int64, int64>> vertices;
    vertices.reserve(N);

    for (int i = 0; i < N; ++i) {
        const auto &previous = original[(i + N - 1) % N];
        const auto &current = original[i];
        const auto &next = original[(i + 1) % N];

        bool collinear =
            (previous.first == current.first &&
             current.first == next.first) ||
            (previous.second == current.second &&
             current.second == next.second);

        if (!collinear) vertices.push_back(current);
    }

    map<int64, vector<pair<int64, int64>>> events;
    int V = static_cast<int>(vertices.size());

    for (int i = 0; i < V; ++i) {
        auto [x1, y1] = vertices[i];
        auto [x2, y2] = vertices[(i + 1) % V];

        if (x1 == x2) {
            events[x1].push_back({min(y1, y2), max(y1, y2)});
        }
    }

    for (auto &[x, edges] : events) {
        sort(edges.begin(), edges.end());
    }

    DSU dsu(N + 5);
    int nextComponent = 0;

    // Cross-section immediately to the right of the current sweep coordinate.
    // Intervals belonging to the same accumulated connected component share
    // a DSU component.
    map<int64, Segment> activeSegments;

    auto eraseSegment = [&](map<int64, Segment>::iterator it) {
        dsu.changeActive(it->second.component, -1);
        activeSegments.erase(it);
    };

    auto insertSegment = [&](int64 left, int64 right, int component) {
        if (left >= right) return;
        activeSegments.emplace(left, Segment{right, component});
        dsu.changeActive(component, 1);
    };

    auto containingSegment = [&](int64 left, int64 right) {
        auto it = activeSegments.upper_bound(left);
        if (it == activeSegments.begin()) return activeSegments.end();
        --it;
        if (it->first <= left && it->second.right >= right) return it;
        return activeSegments.end();
    };

    auto coalesceAround = [&](int64 left) {
        auto current = activeSegments.find(left);
        if (current == activeSegments.end()) return;

        if (current != activeSegments.begin()) {
            auto previous = prev(current);
            if (previous->second.right == current->first) {
                int64 mergedLeft = previous->first;
                int64 mergedRight = current->second.right;
                int a = previous->second.component;
                int b = current->second.component;

                eraseSegment(current);
                eraseSegment(previous);

                int root = dsu.unite(a, b);
                insertSegment(mergedLeft, mergedRight, root);
                current = activeSegments.find(mergedLeft);
            }
        }

        auto next = std::next(current);
        if (next != activeSegments.end() &&
            current->second.right == next->first) {
            int64 mergedLeft = current->first;
            int64 mergedRight = next->second.right;
            int a = current->second.component;
            int b = next->second.component;

            eraseSegment(next);
            eraseSegment(current);

            int root = dsu.unite(a, b);
            insertSegment(mergedLeft, mergedRight, root);
        }
    };

    vector<int64> eventX;
    eventX.reserve(events.size());
    for (const auto &[x, edges] : events) eventX.push_back(x);

    int64 answer = 0;

    for (int eventIndex = 0; eventIndex < (int)eventX.size(); ++eventIndex) {
        int64 currentX = eventX[eventIndex];

        for (auto [low, high] : events[currentX]) {
            auto containing = containingSegment(low, high);

            if (containing != activeSegments.end()) {
                int64 oldLeft = containing->first;
                int64 oldRight = containing->second.right;
                int component = containing->second.component;

                dsu.addXConstraint(component, int(currentX & 1LL));
                dsu.addYConstraint(component, int(low & 1LL));
                dsu.addYConstraint(component, int(high & 1LL));

                eraseSegment(containing);
                insertSegment(oldLeft, low, component);
                insertSegment(high, oldRight, component);
            } else {
                int component = nextComponent++;

                dsu.addXConstraint(component, int(currentX & 1LL));
                dsu.addYConstraint(component, int(low & 1LL));
                dsu.addYConstraint(component, int(high & 1LL));

                insertSegment(low, high, component);
                coalesceAround(low);
            }
        }

        if (eventIndex + 1 == (int)eventX.size()) continue;

        int64 nextX = eventX[eventIndex + 1];

        if (dsu.badComponents != 0) continue;
        if (dsu.activeByParity[0] > 0 && dsu.activeByParity[1] > 0) continue;

        int requiredParity = -1;
        if (dsu.activeByParity[0] > 0) requiredParity = 0;
        if (dsu.activeByParity[1] > 0) requiredParity = 1;

        int64 candidate = nextX;
        if (requiredParity != -1 &&
            (candidate & 1LL) != requiredParity) {
            --candidate;
        }

        if (candidate >= currentX + 1) {
            answer = max(answer, candidate);
        }
    }

    cout << answer << '\n';
    return 0;
}