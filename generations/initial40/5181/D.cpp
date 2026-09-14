#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

int n, m, queryCount;
vector<vector<int>> graph;
vector<int> depthNode, firstOccurrence, logTable;
vector<vector<int>> sparseTable;

vector<int64> currentEvents;
vector<vector<pair<int, int64>>> eventHistory;

void buildLCA() {
    depthNode.assign(n + 1, 0);
    firstOccurrence.assign(n + 1, -1);

    vector<int> parent(n + 1, 0);
    vector<int> nextEdge(n + 1, 0);
    vector<int> stack;
    vector<int> euler;
    euler.reserve(2 * n - 1);

    parent[1] = 1;
    stack.push_back(1);
    firstOccurrence[1] = 0;
    euler.push_back(1);

    while (!stack.empty()) {
        int node = stack.back();

        if (nextEdge[node] < static_cast<int>(graph[node].size())) {
            int next = graph[node][nextEdge[node]++];

            if (next == parent[node])
                continue;

            parent[next] = node;
            depthNode[next] = depthNode[node] + 1;
            firstOccurrence[next] = static_cast<int>(euler.size());
            euler.push_back(next);
            stack.push_back(next);
        } else {
            stack.pop_back();
            if (!stack.empty())
                euler.push_back(stack.back());
        }
    }

    int length = static_cast<int>(euler.size());

    logTable.assign(length + 1, 0);
    for (int i = 2; i <= length; ++i)
        logTable[i] = logTable[i / 2] + 1;

    int levels = logTable[length] + 1;
    sparseTable.assign(levels, vector<int>(length));
    sparseTable[0] = euler;

    for (int level = 1; level < levels; ++level) {
        int block = 1 << level;
        int half = block >> 1;

        for (int i = 0; i + block <= length; ++i) {
            int a = sparseTable[level - 1][i];
            int b = sparseTable[level - 1][i + half];
            sparseTable[level][i] =
                (depthNode[a] < depthNode[b] ? a : b);
        }
    }
}

int lca(int a, int b) {
    int left = firstOccurrence[a];
    int right = firstOccurrence[b];

    if (left > right)
        swap(left, right);

    int length = right - left + 1;
    int level = logTable[length];

    int x = sparseTable[level][left];
    int y = sparseTable[level][right - (1 << level) + 1];

    return depthNode[x] < depthNode[y] ? x : y;
}

int treeDistance(int a, int b) {
    int ancestor = lca(a, b);
    return depthNode[a] + depthNode[b] - 2 * depthNode[ancestor];
}

int64 eventValueAt(int city, int time) {
    const auto& history = eventHistory[city];

    auto it = upper_bound(
        history.begin(), history.end(),
        make_pair(time, numeric_limits<int64>::max())
    );

    if (it == history.begin())
        return 0;

    return prev(it)->second;
}

class TouristSegmentTree {
    struct Tag {
        int firstTime = -1;
        int firstCity = 0;
        int lastCity = 0;
        int64 constant = 0;

        bool exists() const {
            return firstTime != -1;
        }

        void clear() {
            firstTime = -1;
            firstCity = 0;
            lastCity = 0;
            constant = 0;
        }
    };

    int size;
    vector<Tag> lazy;
    vector<int> city;
    vector<int64> adjustedOpinion;

    void appendTag(Tag& destination, const Tag& source) {
        if (!source.exists())
            return;

        if (!destination.exists()) {
            destination = source;
            return;
        }

        int intermediateCity = destination.lastCity;

        destination.constant +=
            eventValueAt(intermediateCity, source.firstTime)
            - treeDistance(intermediateCity, source.firstCity)
            + source.constant;

        destination.lastCity = source.lastCity;
    }

    void appendTravel(Tag& tag, int time, int destinationCity) {
        if (!tag.exists()) {
            tag.firstTime = time;
            tag.firstCity = destinationCity;
            tag.lastCity = destinationCity;
            tag.constant = -currentEvents[destinationCity];
            return;
        }

        tag.constant +=
            currentEvents[tag.lastCity]
            - treeDistance(tag.lastCity, destinationCity)
            - currentEvents[destinationCity];

        tag.lastCity = destinationCity;
    }

    void push(int node) {
        if (!lazy[node].exists())
            return;

        appendTag(lazy[node * 2], lazy[node]);
        appendTag(lazy[node * 2 + 1], lazy[node]);
        lazy[node].clear();
    }

    void materialize(int node, int position) {
        if (!lazy[node].exists())
            return;

        const Tag& tag = lazy[node];

        adjustedOpinion[position] +=
            eventValueAt(city[position], tag.firstTime)
            - treeDistance(city[position], tag.firstCity)
            + tag.constant;

        city[position] = tag.lastCity;
        lazy[node].clear();
    }

    void rangeTravel(
        int node, int left, int right,
        int queryLeft, int queryRight,
        int time, int destinationCity
    ) {
        if (queryLeft <= left && right <= queryRight) {
            appendTravel(lazy[node], time, destinationCity);
            return;
        }

        push(node);

        int middle = left + (right - left) / 2;

        if (queryLeft <= middle) {
            rangeTravel(
                node * 2, left, middle,
                queryLeft, queryRight,
                time, destinationCity
            );
        }

        if (queryRight > middle) {
            rangeTravel(
                node * 2 + 1, middle + 1, right,
                queryLeft, queryRight,
                time, destinationCity
            );
        }
    }

    int64 pointQuery(int node, int left, int right, int position) {
        if (left == right) {
            materialize(node, position);
            return adjustedOpinion[position] + currentEvents[city[position]];
        }

        push(node);

        int middle = left + (right - left) / 2;

        if (position <= middle)
            return pointQuery(node * 2, left, middle, position);

        return pointQuery(node * 2 + 1, middle + 1, right, position);
    }

public:
    explicit TouristSegmentTree(const vector<int>& initialCities)
        : size(static_cast<int>(initialCities.size()) - 1),
          lazy(4 * size + 5),
          city(initialCities),
          adjustedOpinion(size + 1, 0) {}

    void travel(int left, int right, int time, int destinationCity) {
        rangeTravel(
            1, 1, size, left, right,
            time, destinationCity
        );
    }

    int64 opinion(int tourist) {
        return pointQuery(1, 1, size, tourist);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n >> m >> queryCount;

    vector<int> initialCities(m + 1);
    for (int i = 1; i <= m; ++i)
        cin >> initialCities[i];

    graph.assign(n + 1, {});
    for (int i = 0; i < n - 1; ++i) {
        int a, b;
        cin >> a >> b;
        graph[a].push_back(b);
        graph[b].push_back(a);
    }

    buildLCA();

    currentEvents.assign(n + 1, 0);
    eventHistory.assign(n + 1, {});

    TouristSegmentTree tourists(initialCities);

    for (int time = 1; time <= queryCount; ++time) {
        char type;
        cin >> type;

        if (type == 't') {
            int left, right, destination;
            cin >> left >> right >> destination;
            tourists.travel(left, right, time, destination);
        } else if (type == 'e') {
            int city;
            int64 value;
            cin >> city >> value;

            currentEvents[city] += value;
            eventHistory[city].push_back({time, currentEvents[city]});
        } else {
            int tourist;
            cin >> tourist;
            cout << tourists.opinion(tourist) << '\n';
        }
    }

    return 0;
}