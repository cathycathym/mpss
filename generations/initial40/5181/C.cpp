#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
using namespace std;

using int64 = long long;

int n, m, qn;
vector<vector<int>> graphTree;
vector<int> depthNode, firstOccurrence, logTable;
vector<vector<int>> sparseTable;

vector<int64> currentEvents;
vector<vector<pair<int, int64>>> eventHistory;

int lca(int u, int v) {
    int left = firstOccurrence[u];
    int right = firstOccurrence[v];
    if (left > right) swap(left, right);

    int length = right - left + 1;
    int k = logTable[length];

    int a = sparseTable[k][left];
    int b = sparseTable[k][right];
    return depthNode[a] < depthNode[b] ? a : b;
}

int treeDistance(int u, int v) {
    int w = lca(u, v);
    return depthNode[u] + depthNode[v] - 2 * depthNode[w];
}

int64 eventValueAt(int city, int time) {
    const vector<pair<int, int64>>& history = eventHistory[city];

    int low = 0;
    int high = static_cast<int>(history.size());

    while (low < high) {
        int mid = low + (high - low) / 2;
        if (history[mid].first <= time) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }

    return low == 0 ? 0LL : history[low - 1].second;
}

void buildLCA() {
    depthNode.assign(n + 1, 0);
    firstOccurrence.assign(n + 1, -1);

    vector<int> parent(n + 1, 0);
    vector<int> nextEdge(n + 1, 0);
    vector<int> stack;
    vector<int> euler;
    euler.reserve(2 * n - 1);

    stack.push_back(1);
    firstOccurrence[1] = 0;
    euler.push_back(1);

    while (!stack.empty()) {
        int v = stack.back();

        if (nextEdge[v] < static_cast<int>(graphTree[v].size())) {
            int to = graphTree[v][nextEdge[v]++];
            if (to == parent[v]) continue;

            parent[to] = v;
            depthNode[to] = depthNode[v] + 1;
            firstOccurrence[to] = static_cast<int>(euler.size());
            euler.push_back(to);
            stack.push_back(to);
        } else {
            stack.pop_back();
            if (!stack.empty()) {
                euler.push_back(stack.back());
            }
        }
    }

    int length = static_cast<int>(euler.size());
    logTable.assign(length + 1, 0);
    for (int i = 2; i <= length; ++i) {
        logTable[i] = logTable[i / 2] + 1;
    }

    int levels = logTable[length] + 1;
    sparseTable.assign(levels, vector<int>(length));
    sparseTable[0] = euler;

    for (int k = 1; k < levels; ++k) {
        int half = 1 << (k - 1);
        int full = 1 << k;

        for (int i = 0; i + full <= length; ++i) {
            int a = sparseTable[k - 1][i];
            int b = sparseTable[k - 1][i + half];
            sparseTable[k][i] =
                depthNode[a] < depthNode[b] ? a : b;
        }
    }
}

class TouristSegmentTree {
private:
    int size;

    // A tag represents:
    // S' = S + H[firstTime](oldCity)
    //          - dist(oldCity, firstCity) + addedConstant
    // final city = lastCity.
    vector<int> firstTime;
    vector<int> firstCity;
    vector<int> lastCity;
    vector<int64> addedConstant;

    vector<int> touristCity;
    vector<int64> adjustedOpinion;

    bool hasTag(int node) const {
        return firstTime[node] != -1;
    }

    void clearTag(int node) {
        firstTime[node] = -1;
        firstCity[node] = 0;
        lastCity[node] = 0;
        addedConstant[node] = 0;
    }

    void copyTag(int destination, int source) {
        firstTime[destination] = firstTime[source];
        firstCity[destination] = firstCity[source];
        lastCity[destination] = lastCity[source];
        addedConstant[destination] = addedConstant[source];
    }

    // Append the complete transformation in source after the
    // transformation already stored in destination.
    void appendTag(int destination, int source) {
        if (!hasTag(source)) return;

        if (!hasTag(destination)) {
            copyTag(destination, source);
            return;
        }

        int intermediateCity = lastCity[destination];

        addedConstant[destination] +=
            eventValueAt(intermediateCity, firstTime[source])
            - treeDistance(intermediateCity, firstCity[source])
            + addedConstant[source];

        lastCity[destination] = lastCity[source];
    }

    // Append one travel operation at the current query time.
    void appendTravel(int node, int time, int destinationCity) {
        if (!hasTag(node)) {
            firstTime[node] = time;
            firstCity[node] = destinationCity;
            lastCity[node] = destinationCity;
            addedConstant[node] = -currentEvents[destinationCity];
            return;
        }

        int oldLastCity = lastCity[node];

        addedConstant[node] +=
            currentEvents[oldLastCity]
            - treeDistance(oldLastCity, destinationCity)
            - currentEvents[destinationCity];

        lastCity[node] = destinationCity;
    }

    void push(int node) {
        if (!hasTag(node)) return;

        appendTag(node * 2, node);
        appendTag(node * 2 + 1, node);
        clearTag(node);
    }

    void materializeLeaf(int node, int position) {
        if (!hasTag(node)) return;

        int oldCity = touristCity[position];

        adjustedOpinion[position] +=
            eventValueAt(oldCity, firstTime[node])
            - treeDistance(oldCity, firstCity[node])
            + addedConstant[node];

        touristCity[position] = lastCity[node];
        clearTag(node);
    }

    void rangeTravel(int node, int left, int right,
                     int queryLeft, int queryRight,
                     int time, int destinationCity) {
        if (queryLeft <= left && right <= queryRight) {
            appendTravel(node, time, destinationCity);
            return;
        }

        push(node);

        int middle = left + (right - left) / 2;
        if (queryLeft <= middle) {
            rangeTravel(node * 2, left, middle,
                        queryLeft, queryRight,
                        time, destinationCity);
        }
        if (queryRight > middle) {
            rangeTravel(node * 2 + 1, middle + 1, right,
                        queryLeft, queryRight,
                        time, destinationCity);
        }
    }

    int64 pointOpinion(int node, int left, int right, int position) {
        if (left == right) {
            materializeLeaf(node, position);
            return adjustedOpinion[position]
                   + currentEvents[touristCity[position]];
        }

        push(node);

        int middle = left + (right - left) / 2;
        if (position <= middle) {
            return pointOpinion(node * 2, left, middle, position);
        }
        return pointOpinion(node * 2 + 1, middle + 1, right, position);
    }

public:
    explicit TouristSegmentTree(const vector<int>& initialCities) {
        size = static_cast<int>(initialCities.size()) - 1;

        int treeSize = 4 * size + 5;
        firstTime.assign(treeSize, -1);
        firstCity.assign(treeSize, 0);
        lastCity.assign(treeSize, 0);
        addedConstant.assign(treeSize, 0);

        touristCity = initialCities;
        adjustedOpinion.assign(size + 1, 0);
    }

    void travel(int left, int right, int time, int destinationCity) {
        rangeTravel(1, 1, size, left, right, time, destinationCity);
    }

    int64 opinion(int tourist) {
        return pointOpinion(1, 1, size, tourist);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n >> m >> qn;

    vector<int> initialCities(m + 1);
    for (int i = 1; i <= m; ++i) {
        cin >> initialCities[i];
    }

    graphTree.assign(n + 1, {});
    for (int i = 0; i < n - 1; ++i) {
        int u, v;
        cin >> u >> v;
        graphTree[u].push_back(v);
        graphTree[v].push_back(u);
    }

    buildLCA();

    currentEvents.assign(n + 1, 0);
    eventHistory.assign(n + 1, {});

    TouristSegmentTree tourists(initialCities);

    for (int time = 1; time <= qn; ++time) {
        char type;
        cin >> type;

        if (type == 't') {
            int firstTourist, lastTourist, city;
            cin >> firstTourist >> lastTourist >> city;
            tourists.travel(firstTourist, lastTourist, time, city);
        } else if (type == 'e') {
            int city;
            int64 value;
            cin >> city >> value;

            currentEvents[city] += value;
            eventHistory[city].push_back(
                {time, currentEvents[city]}
            );
        } else {
            int tourist;
            cin >> tourist;
            cout << tourists.opinion(tourist) << '\n';
        }
    }

    return 0;
}