#include <bits/stdc++.h>
using namespace std;

constexpr int INF = 1'000'000'000;

struct QueryData {
    vector<pair<int, int>> byLeft, byRight;
    vector<int> maximumRight, minimumLeft;
    int base = 1;
};

int N, M;
vector<int> leftEnd, rightEnd, answer;
vector<char> active;
vector<QueryData> queryTree;
vector<vector<int>> positionTree;

void addQuery(int node, int low, int high, int index) {
    queryTree[node].byLeft.push_back({leftEnd[index], index});
    queryTree[node].byRight.push_back({rightEnd[index], index});

    if (low == high) return;

    int middle = (low + high) / 2;
    if (index <= middle)
        addQuery(node * 2, low, middle, index);
    else
        addQuery(node * 2 + 1, middle + 1, high, index);
}

void buildQueryTree(int node, int low, int high) {
    QueryData &data = queryTree[node];

    sort(data.byLeft.begin(), data.byLeft.end());
    sort(data.byRight.begin(), data.byRight.end());

    int size = static_cast<int>(data.byLeft.size());
    while (data.base < size) data.base *= 2;

    data.maximumRight.assign(data.base * 2, -INF);
    data.minimumLeft.assign(data.base * 2, INF);

    for (int i = 0; i < size; ++i) {
        int leftIndex = data.byLeft[i].second;
        int rightIndex = data.byRight[i].second;

        data.maximumRight[data.base + i] = rightEnd[leftIndex];
        data.minimumLeft[data.base + i] = leftEnd[rightIndex];
    }

    for (int i = data.base - 1; i >= 1; --i) {
        data.maximumRight[i] =
            max(data.maximumRight[i * 2], data.maximumRight[i * 2 + 1]);
        data.minimumLeft[i] =
            min(data.minimumLeft[i * 2], data.minimumLeft[i * 2 + 1]);
    }

    if (low == high) return;

    int middle = (low + high) / 2;
    buildQueryTree(node * 2, low, middle);
    buildQueryTree(node * 2 + 1, middle + 1, high);
}

int rangeMaximum(const QueryData &data, int left, int right) {
    int result = -INF;

    for (left += data.base, right += data.base;
         left < right;
         left /= 2, right /= 2) {
        if (left & 1) result = max(result, data.maximumRight[left++]);
        if (right & 1) result = max(result, data.maximumRight[--right]);
    }

    return result;
}

int rangeMinimum(const QueryData &data, int left, int right) {
    int result = INF;

    for (left += data.base, right += data.base;
         left < right;
         left /= 2, right /= 2) {
        if (left & 1) result = min(result, data.minimumLeft[left++]);
        if (right & 1) result = min(result, data.minimumLeft[--right]);
    }

    return result;
}

void eraseFromNode(QueryData &data, int index) {
    int position = lower_bound(
        data.byLeft.begin(), data.byLeft.end(),
        make_pair(leftEnd[index], index)
    ) - data.byLeft.begin();

    position += data.base;
    data.maximumRight[position] = -INF;

    for (position /= 2; position >= 1; position /= 2) {
        data.maximumRight[position] = max(
            data.maximumRight[position * 2],
            data.maximumRight[position * 2 + 1]
        );
        if (position == 1) break;
    }

    position = lower_bound(
        data.byRight.begin(), data.byRight.end(),
        make_pair(rightEnd[index], index)
    ) - data.byRight.begin();

    position += data.base;
    data.minimumLeft[position] = INF;

    for (position /= 2; position >= 1; position /= 2) {
        data.minimumLeft[position] = min(
            data.minimumLeft[position * 2],
            data.minimumLeft[position * 2 + 1]
        );
        if (position == 1) break;
    }
}

void eraseQuery(int node, int low, int high, int index) {
    eraseFromNode(queryTree[node], index);

    if (low == high) return;

    int middle = (low + high) / 2;
    if (index <= middle)
        eraseQuery(node * 2, low, middle, index);
    else
        eraseQuery(node * 2 + 1, middle + 1, high, index);
}

bool containsPartialInterval(int node, int firstFree, int lastFree) {
    const QueryData &data = queryTree[node];

    int begin = lower_bound(
        data.byLeft.begin(), data.byLeft.end(),
        make_pair(firstFree + 1, -INF)
    ) - data.byLeft.begin();

    int end = upper_bound(
        data.byLeft.begin(), data.byLeft.end(),
        make_pair(lastFree, INF)
    ) - data.byLeft.begin();

    if (begin < end &&
        rangeMaximum(data, begin, end) >= firstFree) {
        return true;
    }

    begin = lower_bound(
        data.byRight.begin(), data.byRight.end(),
        make_pair(firstFree, -INF)
    ) - data.byRight.begin();

    end = upper_bound(
        data.byRight.begin(), data.byRight.end(),
        make_pair(lastFree - 1, INF)
    ) - data.byRight.begin();

    return begin < end &&
           rangeMinimum(data, begin, end) <= lastFree;
}

int findFirstPartial(int node, int low, int high,
                     int firstFree, int lastFree) {
    if (!containsPartialInterval(node, firstFree, lastFree))
        return -1;

    if (low == high) return low;

    int middle = (low + high) / 2;

    int result = findFirstPartial(
        node * 2, low, middle, firstFree, lastFree
    );
    if (result != -1) return result;

    return findFirstPartial(
        node * 2 + 1, middle + 1, high, firstFree, lastFree
    );
}

void addPositionInterval(int node, int low, int high,
                         int queryLeft, int queryRight, int index) {
    if (queryLeft <= low && high <= queryRight) {
        positionTree[node].push_back(index);
        return;
    }

    int middle = (low + high) / 2;

    if (queryLeft <= middle)
        addPositionInterval(
            node * 2, low, middle, queryLeft, queryRight, index
        );

    if (queryRight > middle)
        addPositionInterval(
            node * 2 + 1, middle + 1, high,
            queryLeft, queryRight, index
        );
}

void coverPosition(int node, int low, int high,
                   int position, int beauty) {
    vector<int> queries;
    queries.swap(positionTree[node]);

    for (int index : queries) {
        if (!active[index]) continue;

        active[index] = false;
        answer[index] = beauty;
        eraseQuery(1, 1, M, index);
    }

    if (low == high) return;

    int middle = (low + high) / 2;
    if (position <= middle)
        coverPosition(node * 2, low, middle, position, beauty);
    else
        coverPosition(node * 2 + 1, middle + 1, high, position, beauty);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> N >> M;

    vector<int> beauty(N);
    for (int &value : beauty) cin >> value;
    sort(beauty.rbegin(), beauty.rend());

    leftEnd.assign(M + 1, 0);
    rightEnd.assign(M + 1, 0);
    answer.assign(M + 1, 0);
    active.assign(M + 1, true);

    for (int i = 1; i <= M; ++i)
        cin >> leftEnd[i] >> rightEnd[i];

    queryTree.resize(4 * M + 5);

    for (int i = 1; i <= M; ++i)
        addQuery(1, 1, M, i);

    buildQueryTree(1, 1, M);

    positionTree.resize(4 * N + 5);

    for (int i = 1; i <= M; ++i) {
        addPositionInterval(
            1, 1, N, leftEnd[i], rightEnd[i], i
        );
    }

    set<int> freePositions;
    for (int position = 1; position <= N; ++position)
        freePositions.insert(position);

    int beautyIndex = 0;

    for (int i = 1; i <= M; ++i) {
        if (!active[i]) continue;

        int feasibleLeft = leftEnd[i];
        int feasibleRight = rightEnd[i];

        while (true) {
            auto firstIterator =
                freePositions.lower_bound(feasibleLeft);
            auto afterLastIterator =
                freePositions.upper_bound(feasibleRight);

            int firstFree = *firstIterator;
            int lastFree = *prev(afterLastIterator);

            int query = findFirstPartial(
                1, 1, M, firstFree, lastFree
            );

            if (query == -1) break;

            feasibleLeft = max(feasibleLeft, leftEnd[query]);
            feasibleRight = min(feasibleRight, rightEnd[query]);
        }

        int position =
            *freePositions.lower_bound(feasibleLeft);
        freePositions.erase(position);

        coverPosition(
            1, 1, N, position, beauty[beautyIndex++]
        );
    }

    for (int i = 1; i <= M; ++i)
        cout << answer[i] << '\n';

    return 0;
}