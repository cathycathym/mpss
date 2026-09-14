#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

class LevelAncestorForest {
public:
    vector<int> parent;
    vector<int64> distanceToRoot;

private:
    int n, logN;
    vector<vector<int>> up;
    vector<int> ladderId, ladderPos;
    vector<vector<int>> ladders;

public:
    LevelAncestorForest(vector<int> p, vector<int64> edgeWeight)
        : parent(std::move(p)) {
        n = (int)parent.size();

        vector<vector<int>> children(n);
        vector<int> roots;
        roots.reserve(n);

        for (int v = 0; v < n; ++v) {
            if (parent[v] == -1) {
                roots.push_back(v);
            } else {
                children[parent[v]].push_back(v);
            }
        }

        vector<int> order;
        order.reserve(n);
        for (int root : roots) order.push_back(root);

        distanceToRoot.assign(n, 0);
        for (size_t i = 0; i < order.size(); ++i) {
            int v = order[i];
            for (int child : children[v]) {
                distanceToRoot[child] =
                    distanceToRoot[v] + edgeWeight[child];
                order.push_back(child);
            }
        }

        vector<int> height(n, 0), heavyChild(n, -1);
        for (int i = n - 1; i >= 0; --i) {
            int v = order[i];
            for (int child : children[v]) {
                if (height[child] + 1 > height[v]) {
                    height[v] = height[child] + 1;
                    heavyChild[v] = child;
                }
            }
        }

        logN = 1;
        while ((1 << logN) <= max(1, n)) ++logN;

        up.assign(logN, vector<int>(n, -1));
        up[0] = parent;
        for (int j = 1; j < logN; ++j) {
            for (int v = 0; v < n; ++v) {
                int middle = up[j - 1][v];
                if (middle != -1) {
                    up[j][v] = up[j - 1][middle];
                }
            }
        }

        ladderId.assign(n, -1);
        ladderPos.assign(n, -1);

        for (int head = 0; head < n; ++head) {
            if (parent[head] != -1 &&
                heavyChild[parent[head]] == head) {
                continue;
            }

            vector<int> mainPath;
            for (int v = head; v != -1; v = heavyChild[v]) {
                mainPath.push_back(v);
            }

            int length = (int)mainPath.size();
            vector<int> extension;
            int v = parent[head];

            for (int i = 0; i < length && v != -1; ++i) {
                extension.push_back(v);
                v = parent[v];
            }
            reverse(extension.begin(), extension.end());

            vector<int> ladder;
            ladder.reserve(extension.size() + mainPath.size());
            for (int x : extension) ladder.push_back(x);
            for (int x : mainPath) ladder.push_back(x);

            int id = (int)ladders.size();
            int offset = (int)extension.size();
            ladders.push_back(std::move(ladder));

            for (int i = 0; i < length; ++i) {
                ladderId[mainPath[i]] = id;
                ladderPos[mainPath[i]] = offset + i;
            }
        }
    }

    int ancestor(int v, int steps) const {
        if (steps == 0) return v;

        int j = 31 - __builtin_clz(steps);
        int u = up[j][v];
        int remaining = steps - (1 << j);

        return ladders[ladderId[u]][ladderPos[u] - remaining];
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    int64 T;
    cin >> N >> T;

    int edgeCount = N - 1;
    vector<vector<int>> flights(edgeCount);
    vector<int64> departure, arrival;

    for (int edge = 0; edge < edgeCount; ++edge) {
        int M;
        cin >> M;
        flights[edge].reserve(M);

        for (int j = 0; j < M; ++j) {
            int64 A, B;
            cin >> A >> B;

            int id = (int)departure.size();
            departure.push_back(A);
            arrival.push_back(B);
            flights[edge].push_back(id);
        }
    }

    int totalFlights = (int)departure.size();

    vector<int> forwardParent(totalFlights, -1);
    vector<int64> forwardWeight(totalFlights, 0);

    for (int edge = 0; edge + 1 < edgeCount; ++edge) {
        vector<int> ordered = flights[edge + 1];
        sort(ordered.begin(), ordered.end(),
             [&](int x, int y) {
                 if (departure[x] != departure[y])
                     return departure[x] < departure[y];
                 return arrival[x] < arrival[y];
             });

        int m = (int)ordered.size();
        vector<int> suffixBest(m);
        suffixBest[m - 1] = ordered[m - 1];

        for (int i = m - 2; i >= 0; --i) {
            int x = ordered[i];
            int y = suffixBest[i + 1];
            suffixBest[i] = (arrival[x] <= arrival[y] ? x : y);
        }

        for (int current : flights[edge]) {
            int position = (int)(
                lower_bound(
                    ordered.begin(), ordered.end(), arrival[current],
                    [&](int id, int64 readyTime) {
                        return departure[id] < readyTime;
                    }
                ) - ordered.begin()
            );

            bool nextDay = (position == m);
            int nextFlight = nextDay
                ? suffixBest[0]
                : suffixBest[position];

            forwardParent[current] = nextFlight;
            forwardWeight[current] =
                arrival[nextFlight] - arrival[current]
                + (nextDay ? T : 0);
        }
    }

    vector<int> backwardParent(totalFlights, -1);
    vector<int64> backwardWeight(totalFlights, 0);

    for (int edge = edgeCount - 1; edge >= 1; --edge) {
        vector<int> ordered = flights[edge - 1];
        sort(ordered.begin(), ordered.end(),
             [&](int x, int y) {
                 if (arrival[x] != arrival[y])
                     return arrival[x] < arrival[y];
                 return departure[x] < departure[y];
             });

        int m = (int)ordered.size();
        vector<int> prefixBest(m);
        prefixBest[0] = ordered[0];

        for (int i = 1; i < m; ++i) {
            int x = ordered[i];
            int y = prefixBest[i - 1];
            prefixBest[i] =
                (departure[x] >= departure[y] ? x : y);
        }

        for (int current : flights[edge]) {
            int low = 0, high = m;
            while (low < high) {
                int middle = (low + high) / 2;
                if (arrival[ordered[middle]] <= departure[current]) {
                    low = middle + 1;
                } else {
                    high = middle;
                }
            }

            int eligible = low;
            bool previousDay = (eligible == 0);
            int previousFlight = previousDay
                ? prefixBest[m - 1]
                : prefixBest[eligible - 1];

            backwardParent[current] = previousFlight;
            backwardWeight[current] =
                departure[current] - departure[previousFlight]
                + (previousDay ? T : 0);
        }
    }

    LevelAncestorForest forward(
        std::move(forwardParent), std::move(forwardWeight)
    );
    LevelAncestorForest backward(
        std::move(backwardParent), std::move(backwardWeight)
    );

    int Q;
    cin >> Q;

    unordered_map<unsigned long long, int64> cache;
    cache.reserve((size_t)Q * 2);

    const int64 INF = (1LL << 62);

    for (int query = 0; query < Q; ++query) {
        int L, R;
        cin >> L >> R;

        int firstEdge = L - 1;
        int lastEdge = R - 2;
        int steps = lastEdge - firstEdge;

        unsigned long long key =
            (static_cast<unsigned long long>(
                static_cast<unsigned int>(firstEdge)
             ) << 32)
            | static_cast<unsigned int>(lastEdge);

        auto cached = cache.find(key);
        if (cached != cache.end()) {
            cout << cached->second << '\n';
            continue;
        }

        int64 answer = INF;

        if (flights[firstEdge].size() <= flights[lastEdge].size()) {
            for (int firstFlight : flights[firstEdge]) {
                int lastFlight =
                    forward.ancestor(firstFlight, steps);

                int64 duration =
                    arrival[firstFlight] - departure[firstFlight]
                    + forward.distanceToRoot[firstFlight]
                    - forward.distanceToRoot[lastFlight];

                answer = min(answer, duration);
            }
        } else {
            for (int lastFlight : flights[lastEdge]) {
                int firstFlight =
                    backward.ancestor(lastFlight, steps);

                int64 duration =
                    arrival[lastFlight] - departure[lastFlight]
                    + backward.distanceToRoot[lastFlight]
                    - backward.distanceToRoot[firstFlight];

                answer = min(answer, duration);
            }
        }

        cache.emplace(key, answer);
        cout << answer << '\n';
    }

    return 0;
}
