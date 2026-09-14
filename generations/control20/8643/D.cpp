#include <bits/stdc++.h>
using namespace std;

using ll = long long;

const int INF = 1000000000;
const ll INF64 = (1LL << 62);

struct Line {
    ll slope;
    ll intercept;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M, K;
    cin >> N >> M >> K;

    vector<vector<int> > graph(N);
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
    for (int i = 0; i < K; ++i) {
        cin >> X[i];
        --X[i];
    }

    vector<int> stops;
    for (int v = 0; v < N; ++v) {
        if (S[v] == '1') stops.push_back(v);
    }

    if (stops.empty()) {
        vector<int> dist(N, INF);
        queue<int> q;
        dist[X[0]] = 0;
        q.push(X[0]);

        while (!q.empty()) {
            int v = q.front();
            q.pop();

            for (int i = 0; i < (int)graph[v].size(); ++i) {
                int to = graph[v][i];
                if (dist[to] == INF) {
                    dist[to] = dist[v] + 1;
                    q.push(to);
                }
            }
        }

        for (int v = 0; v < N; ++v) {
            cout << dist[v] << '\n';
        }
        return 0;
    }

    vector<char> good(N, false);
    for (int i = 0; i < (int)stops.size(); ++i) {
        int v = stops[i];
        for (int j = 0; j < (int)graph[v].size(); ++j) {
            int to = graph[v][j];
            if (S[to] == '1') {
                good[v] = true;
                break;
            }
        }
    }

    vector<int> distanceToStop(N, INF);
    queue<int> q;
    for (int i = 0; i < (int)stops.size(); ++i) {
        distanceToStop[stops[i]] = 0;
        q.push(stops[i]);
    }

    while (!q.empty()) {
        int v = q.front();
        q.pop();

        for (int i = 0; i < (int)graph[v].size(); ++i) {
            int to = graph[v][i];
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

    for (int i = 0; i < (int)stops.size(); ++i) {
        int v = stops[i];
        if (good[v]) {
            adjustedDistance[v] = 0;
            dq.push_back(v);
        }
    }

    while (!dq.empty()) {
        int v = dq.front();
        dq.pop_front();

        int weight = (S[v] == '0' ? 1 : 0);
        for (int i = 0; i < (int)graph[v].size(); ++i) {
            int to = graph[v][i];
            int nd = adjustedDistance[v] + weight;

            if (nd < adjustedDistance[to]) {
                adjustedDistance[to] = nd;
                if (weight == 0) {
                    dq.push_front(to);
                } else {
                    dq.push_back(to);
                }
            }
        }
    }

    const int otherPlayers = K - 1;
    ll initialIntercept = 0;
    vector<pair<int, ll> > changes;

    for (int p = 1; p < K; ++p) {
        int first = firstTurnCost[X[p]];
        ll baselineIntercept = first - 2LL;
        initialIntercept += baselineIntercept;

        if (adjustedDistance[X[p]] != INF) {
            ll eventualIntercept = adjustedDistance[X[p]];
            int threshold = (int)(eventualIntercept - first + 2);
            threshold = max(threshold, 1);

            changes.push_back(make_pair(
                threshold,
                eventualIntercept - baselineIntercept
            ));
        }
    }

    sort(changes.begin(), changes.end());

    vector<Line> lines;
    ll slope = 2LL * otherPlayers;
    ll intercept = initialIntercept;
    lines.push_back(Line{slope, intercept});

    for (int i = 0; i < (int)changes.size();) {
        int j = i;
        ll interceptChange = 0;

        while (j < (int)changes.size() &&
               changes[j].first == changes[i].first) {
            interceptChange += changes[j].second;
            ++j;
        }

        slope -= j - i;
        intercept += interceptChange;
        lines.push_back(Line{slope, intercept});
        i = j;
    }

    vector<int> oneTurnDistance(N, INF);
    queue<int> oneTurnQueue;
    oneTurnDistance[X[0]] = 0;
    oneTurnQueue.push(X[0]);

    while (!oneTurnQueue.empty()) {
        int v = oneTurnQueue.front();
        oneTurnQueue.pop();

        for (int i = 0; i < (int)graph[v].size(); ++i) {
            int to = graph[v][i];
            if (oneTurnDistance[to] != INF) continue;

            oneTurnDistance[to] = oneTurnDistance[v] + 1;
            if (S[to] == '0') {
                oneTurnQueue.push(to);
            }
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

        for (int i = 0; i < (int)graph[v].size(); ++i) {
            int to = graph[v][i];
            int weight = (S[to] == '1' ? 1 : 0);
            int nd = minimumStopCount[v] + weight;

            if (nd < minimumStopCount[to]) {
                minimumStopCount[to] = nd;
                if (weight == 0) {
                    countQueue.push_front(to);
                } else {
                    countQueue.push_back(to);
                }
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

        int startId = X[0] * width;
        distance[startId] = 0;
        bfsQueue.push_back(startId);

        for (size_t head = 0; head < bfsQueue.size(); ++head) {
            int state = bfsQueue[head];
            int v = state / width;
            int extra = state % width;

            for (int i = 0; i < (int)graph[v].size(); ++i) {
                int to = graph[v][i];
                int enteredStop = (S[to] == '1' ? 1 : 0);

                int increase =
                    minimumStopCount[v] + enteredStop -
                    minimumStopCount[to];

                int nextExtra = extra + increase;
                if (nextExtra > extraLimit) continue;

                int nextState = to * width + nextExtra;
                if (distance[nextState] == INF) {
                    distance[nextState] = distance[state] + 1;
                    bfsQueue.push_back(nextState);
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

        vector<ll> totalOtherCost(maximumRounds + 1, 0);

        ll currentSlope = 2LL * otherPlayers;
        ll currentIntercept = initialIntercept;
        int changeIndex = 0;

        for (int rounds = 1; rounds <= maximumRounds; ++rounds) {
            while (changeIndex < (int)changes.size() &&
                   changes[changeIndex].first == rounds) {
                --currentSlope;
                currentIntercept += changes[changeIndex].second;
                ++changeIndex;
            }

            totalOtherCost[rounds] =
                currentSlope * rounds + currentIntercept;
        }

        for (int v = 0; v < N; ++v) {
            for (int extra = 0; extra <= extraLimit; ++extra) {
                int d = distance[v * width + extra];
                if (d == INF) continue;

                int enteredStops = minimumStopCount[v] + extra;
                int rounds =
                    enteredStops - (S[v] == '1' ? 1 : 0);

                if (rounds < 0) continue;

                answer[v] = min(
                    answer[v],
                    (ll)d + totalOtherCost[rounds]
                );
            }
        }
    } else {
        const int stateCount = 3 * N;

        for (int lineIndex = 0;
             lineIndex < (int)lines.size();
             ++lineIndex) {
            const Line &line = lines[lineIndex];

            vector<ll> dist(stateCount, INF64);
            priority_queue<
                pair<ll, int>,
                vector<pair<ll, int> >,
                greater<pair<ll, int> >
            > pq;

            int startState = X[0];
            dist[startState] = 0;
            pq.push(make_pair(0LL, startState));

            while (!pq.empty()) {
                pair<ll, int> top = pq.top();
                pq.pop();

                ll currentDistance = top.first;
                int state = top.second;

                if (currentDistance != dist[state]) continue;

                int stopState = state / N;
                int v = state % N;

                for (int i = 0; i < (int)graph[v].size(); ++i) {
                    int to = graph[v][i];
                    bool entersStop = (S[to] == '1');

                    int nextStopState = stopState;
                    if (entersStop) {
                        nextStopState = min(2, stopState + 1);
                    }

                    ll edgeCost =
                        1 + (entersStop ? line.slope : 0);

                    int nextState = nextStopState * N + to;
                    ll nd = currentDistance + edgeCost;

                    if (nd < dist[nextState]) {
                        dist[nextState] = nd;
                        pq.push(make_pair(nd, nextState));
                    }
                }
            }

            for (int v = 0; v < N; ++v) {
                ll candidate = INF64;

                if (S[v] == '0') {
                    candidate = min(
                        dist[N + v],
                        dist[2 * N + v]
                    );

                    if (candidate != INF64) {
                        candidate += line.intercept;
                    }
                } else {
                    candidate = dist[2 * N + v];

                    if (candidate != INF64) {
                        candidate +=
                            line.intercept - line.slope;
                    }
                }

                answer[v] = min(answer[v], candidate);
            }
        }
    }

    for (int v = 0; v < N; ++v) {
        cout << answer[v] << '\n';
    }

    return 0;
}
