#include <bits/stdc++.h>
using namespace std;

using ll = long long;
const ll INF64 = (1LL << 62);
const int INF = 1e9;

struct Line {
    ll slope;
    ll intercept;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M, K;
    cin >> N >> M >> K;

    vector<vector<int>> graph(N);
    for (int i = 0; i < M; ++i) {
        int u, v;
        cin >> u >> v;
        --u;
        --v;
        graph[u].push_back(v);
        graph[v].push_back(u);
    }

    string S;
    cin >> S;

    vector<int> X(K);
    for (int &x : X) {
        cin >> x;
        --x;
    }

    vector<int> stops;
    for (int v = 0; v < N; ++v) {
        if (S[v] == '1') stops.push_back(v);
    }

    auto ordinary_bfs = [&](int source) {
        vector<int> dist(N, INF);
        queue<int> q;
        dist[source] = 0;
        q.push(source);

        while (!q.empty()) {
            int v = q.front();
            q.pop();

            for (int to : graph[v]) {
                if (dist[to] == INF) {
                    dist[to] = dist[v] + 1;
                    q.push(to);
                }
            }
        }
        return dist;
    };

    if (stops.empty()) {
        vector<int> answer = ordinary_bfs(X[0]);
        for (int value : answer) cout << value << '\n';
        return 0;
    }

    vector<char> good(N, false);
    for (int v : stops) {
        for (int to : graph[v]) {
            if (S[to] == '1') {
                good[v] = true;
                break;
            }
        }
    }

    vector<int> distanceToStop(N, INF);
    queue<int> q;
    for (int v : stops) {
        distanceToStop[v] = 0;
        q.push(v);
    }

    while (!q.empty()) {
        int v = q.front();
        q.pop();

        for (int to : graph[v]) {
            if (distanceToStop[to] == INF) {
                distanceToStop[to] = distanceToStop[v] + 1;
                q.push(to);
            }
        }
    }

    vector<int> firstTurnCost(N);
    for (int v = 0; v < N; ++v) {
        if (S[v] == '0') {
            firstTurnCost[v] = distanceToStop[v];
        } else {
            firstTurnCost[v] = good[v] ? 1 : 2;
        }
    }

    vector<int> adjustedDistance(N, INF);
    deque<int> dq;

    for (int v : stops) {
        if (good[v]) {
            adjustedDistance[v] = 0;
            dq.push_back(v);
        }
    }

    while (!dq.empty()) {
        int v = dq.front();
        dq.pop_front();

        int weight = (S[v] == '0' ? 1 : 0);
        for (int to : graph[v]) {
            int nd = adjustedDistance[v] + weight;
            if (nd < adjustedDistance[to]) {
                adjustedDistance[to] = nd;
                if (weight == 0) dq.push_front(to);
                else dq.push_back(to);
            }
        }
    }

    int otherPlayers = K - 1;
    ll initialIntercept = 0;
    vector<pair<int, ll>> changes;

    for (int p = 1; p < K; ++p) {
        int d0 = firstTurnCost[X[p]];
        ll baselineIntercept = d0 - 2LL;
        initialIntercept += baselineIntercept;

        if (adjustedDistance[X[p]] != INF) {
            ll c = adjustedDistance[X[p]];
            int threshold = int(c - d0 + 2);
            threshold = max(threshold, 1);
            changes.push_back({threshold, c - baselineIntercept});
        }
    }

    sort(changes.begin(), changes.end());

    vector<Line> lines;
    ll slope = 2LL * otherPlayers;
    ll intercept = initialIntercept;
    lines.push_back({slope, intercept});

    for (int i = 0; i < (int)changes.size();) {
        int j = i;
        ll interceptChange = 0;

        while (j < (int)changes.size() &&
               changes[j].first == changes[i].first) {
            interceptChange += changes[j].second;
            ++j;
        }

        slope -= (j - i);
        intercept += interceptChange;
        lines.push_back({slope, intercept});
        i = j;
    }

    vector<int> oneTurnDistance(N, INF);
    queue<int> oneTurnQueue;
    oneTurnDistance[X[0]] = 0;
    oneTurnQueue.push(X[0]);

    while (!oneTurnQueue.empty()) {
        int v = oneTurnQueue.front();
        oneTurnQueue.pop();

        for (int to : graph[v]) {
            if (oneTurnDistance[to] != INF) continue;

            oneTurnDistance[to] = oneTurnDistance[v] + 1;
            if (S[to] == '0') oneTurnQueue.push(to);
        }
    }

    vector<ll> answer(N, INF64);
    for (int v = 0; v < N; ++v) {
        if (oneTurnDistance[v] != INF) {
            answer[v] = oneTurnDistance[v];
        }
    }
    answer[X[0]] = 0;

    vector<int> minimumStopCount(N, INF);
    deque<int> countQueue;
    minimumStopCount[X[0]] = 0;
    countQueue.push_front(X[0]);

    while (!countQueue.empty()) {
        int v = countQueue.front();
        countQueue.pop_front();

        for (int to : graph[v]) {
            int weight = (S[to] == '1' ? 1 : 0);
            int nd = minimumStopCount[v] + weight;

            if (nd < minimumStopCount[to]) {
                minimumStopCount[to] = nd;
                if (weight == 0) countQueue.push_front(to);
                else countQueue.push_back(to);
            }
        }
    }

    int extraLimit = (N - 1) / otherPlayers;
    bool useExpandedBFS =
        (ll)(extraLimit + 1) <= (ll)lines.size();

    if (useExpandedBFS) {
        int width = extraLimit + 1;
        size_t stateCount = (size_t)N * width;
        vector<int> distance(stateCount, INF);
        vector<int> bfsQueue;
        bfsQueue.reserve(stateCount);

        auto stateId = [&](int v, int extra) -> int {
            return v * width + extra;
        };

        int startId = stateId(X[0], 0);
        distance[startId] = 0;
        bfsQueue.push_back(startId);

        for (size_t head = 0; head < bfsQueue.size(); ++head) {
            int id = bfsQueue[head];
            int v = id / width;
            int extra = id % width;

            for (int to : graph[v]) {
                int enteredStop = (S[to] == '1' ? 1 : 0);
                int increase =
                    minimumStopCount[v] + enteredStop -
                    minimumStopCount[to];
                int nextExtra = extra + increase;

                if (nextExtra > extraLimit) continue;

                int nextId = stateId(to, nextExtra);
                if (distance[nextId] == INF) {
                    distance[nextId] = distance[id] + 1;
                    bfsQueue.push_back(nextId);
                }
            }
        }

        int maximumRounds = 0;
        for (int v = 0; v < N; ++v) {
            maximumRounds = max(
                maximumRounds,
                minimumStopCount[v] + extraLimit
            );
        }

        vector<vector<pair<int, ll>>> events(maximumRounds + 2);
        for (auto [threshold, interceptChange] : changes) {
            if (threshold <= maximumRounds) {
                events[threshold].push_back({1, interceptChange});
            }
        }

        vector<ll> totalOtherCost(maximumRounds + 1, 0);
        ll currentSlope = 2LL * otherPlayers;
        ll currentIntercept = initialIntercept;

        for (int rounds = 1; rounds <= maximumRounds; ++rounds) {
            for (auto [count, interceptChange] : events[rounds]) {
                currentSlope -= count;
                currentIntercept += interceptChange;
            }
            totalOtherCost[rounds] =
                currentSlope * rounds + currentIntercept;
        }

        for (int v = 0; v < N; ++v) {
            for (int extra = 0; extra <= extraLimit; ++extra) {
                int d = distance[stateId(v, extra)];
                if (d == INF) continue;

                int enteredStops = minimumStopCount[v] + extra;
                int rounds = enteredStops - (S[v] == '1' ? 1 : 0);
                if (rounds < 0) continue;

                answer[v] = min(
                    answer[v],
                    (ll)d + totalOtherCost[rounds]
                );
            }
        }
    } else {
        int stateN = 3 * N;

        for (const Line &line : lines) {
            vector<ll> dist(stateN, INF64);
            priority_queue<
                pair<ll, int>,
                vector<pair<ll, int>>,
                greater<pair<ll, int>>
            > pq;

            auto id = [&](int v, int stopState) {
                return stopState * N + v;
            };

            dist[id(X[0], 0)] = 0;
            pq.push({0, id(X[0], 0)});

            while (!pq.empty()) {
                auto [currentDistance, state] = pq.top();
                pq.pop();

                if (currentDistance != dist[state]) continue;

                int stopState = state / N;
                int v = state % N;

                for (int to : graph[v]) {
                    bool entersStop = (S[to] == '1');
                    int nextStopState =
                        entersStop ? min(2, stopState + 1) : stopState;
                    ll edgeCost = 1 + (entersStop ? line.slope : 0);
                    int nextState = id(to, nextStopState);
                    ll nd = currentDistance + edgeCost;

                    if (nd < dist[nextState]) {
                        dist[nextState] = nd;
                        pq.push({nd, nextState});
                    }
                }
            }

            for (int v = 0; v < N; ++v) {
                ll candidate = INF64;

                if (S[v] == '0') {
                    candidate = min(dist[id(v, 1)], dist[id(v, 2)]);
                    if (candidate != INF64) {
                        candidate += line.intercept;
                    }
                } else {
                    candidate = dist[id(v, 2)];
                    if (candidate != INF64) {
                        candidate += line.intercept - line.slope;
                    }
                }

                answer[v] = min(answer[v], candidate);
            }
        }
    }

    for (ll value : answer) {
        cout << value << '\n';
    }

    return 0;
}
