#include <bits/stdc++.h>
using namespace std;

using ll = long long;
const ll INF = (1LL << 62);

struct WalkResult {
    vector<ll> reach;
    vector<ll> arrival;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M, K;
    cin >> N >> M >> K;

    vector<vector<int>> graph(N);
    vector<pair<int,int>> edges;
    edges.reserve(M);

    for (int i = 0; i < M; ++i) {
        int u, v;
        cin >> u >> v;
        --u;
        --v;
        graph[u].push_back(v);
        graph[v].push_back(u);
        edges.push_back({u, v});
    }

    string S;
    cin >> S;

    vector<int> X(K);
    for (int &x : X) {
        cin >> x;
        --x;
    }

    vector<int> stops;
    for (int i = 0; i < N; ++i) {
        if (S[i] == '1') stops.push_back(i);
    }
    const int Z = (int)stops.size();

    auto walk_initial = [&]() -> WalkResult {
        vector<ll> reach(N, INF), arrival(N, INF);
        priority_queue<pair<ll,int>,
                       vector<pair<ll,int>>,
                       greater<pair<ll,int>>> pq;

        int start = X[0];
        reach[start] = 0;

        if (S[start] == '0') {
            pq.push({0, start});
        } else {
            for (int v : graph[start]) {
                if (S[v] == '1') {
                    if (arrival[v] > 1) {
                        arrival[v] = 1;
                        reach[v] = min(reach[v], 1LL);
                    }
                } else if (reach[v] > 1) {
                    reach[v] = 1;
                    pq.push({1, v});
                }
            }
        }

        while (!pq.empty()) {
            auto [du, u] = pq.top();
            pq.pop();
            if (du != reach[u]) continue;

            for (int v : graph[u]) {
                ll nd = du + 1;
                if (S[v] == '1') {
                    if (nd < arrival[v]) {
                        arrival[v] = nd;
                        reach[v] = min(reach[v], nd);
                    }
                } else if (nd < reach[v]) {
                    reach[v] = nd;
                    pq.push({nd, v});
                }
            }
        }

        return {move(reach), move(arrival)};
    };

    auto walk_from_stops = [&](const vector<ll>& source) -> WalkResult {
        vector<ll> reach(N, INF), arrival(N, INF), zeroDist(N, INF);
        priority_queue<pair<ll,int>,
                       vector<pair<ll,int>>,
                       greater<pair<ll,int>>> pq;

        for (int s : stops) {
            if (source[s] >= INF / 4) continue;
            for (int v : graph[s]) {
                ll nd = source[s] + 1;
                if (S[v] == '1') {
                    if (nd < arrival[v]) {
                        arrival[v] = nd;
                        reach[v] = min(reach[v], nd);
                    }
                } else if (nd < zeroDist[v]) {
                    zeroDist[v] = nd;
                    reach[v] = nd;
                    pq.push({nd, v});
                }
            }
        }

        while (!pq.empty()) {
            auto [du, u] = pq.top();
            pq.pop();
            if (du != zeroDist[u]) continue;

            for (int v : graph[u]) {
                ll nd = du + 1;
                if (S[v] == '1') {
                    if (nd < arrival[v]) {
                        arrival[v] = nd;
                        reach[v] = min(reach[v], nd);
                    }
                } else if (nd < zeroDist[v]) {
                    zeroDist[v] = nd;
                    reach[v] = nd;
                    pq.push({nd, v});
                }
            }
        }

        return {move(reach), move(arrival)};
    };

    auto complete_turn_values = [&](const vector<ll>& terminal) {
        vector<ll> zeroDist(N, INF), result(N, INF);
        priority_queue<pair<ll,int>,
                       vector<pair<ll,int>>,
                       greater<pair<ll,int>>> pq;

        for (int s : stops) {
            if (terminal[s] >= INF / 4) continue;
            for (int v : graph[s]) {
                if (S[v] == '0' && terminal[s] + 1 < zeroDist[v]) {
                    zeroDist[v] = terminal[s] + 1;
                    pq.push({zeroDist[v], v});
                }
            }
        }

        while (!pq.empty()) {
            auto [du, u] = pq.top();
            pq.pop();
            if (du != zeroDist[u]) continue;

            for (int v : graph[u]) {
                if (S[v] == '0' && du + 1 < zeroDist[v]) {
                    zeroDist[v] = du + 1;
                    pq.push({zeroDist[v], v});
                }
            }
        }

        for (int v = 0; v < N; ++v) {
            if (S[v] == '0') {
                result[v] = zeroDist[v];
                continue;
            }

            ll best = INF;
            for (int u : graph[v]) {
                if (S[u] == '1') {
                    if (terminal[u] < INF / 4)
                        best = min(best, terminal[u] + 1);
                } else if (zeroDist[u] < INF / 4) {
                    best = min(best, zeroDist[u] + 1);
                }
            }
            result[v] = best;
        }

        return result;
    };

    WalkResult initial = walk_initial();
    vector<ll> answer = initial.reach;

    if (Z == 0) {
        for (ll value : answer) cout << value << '\n';
        return 0;
    }

    /*
       Exact layered solution for small graphs or for at most two stop cells.
    */
    if (N <= 3000 || Z <= 2) {
        vector<ll> terminal(N, INF);
        for (int s : stops) terminal[s] = 0;

        vector<ll> playerPosition = initial.arrival;

        for (int q = 1; q <= Z; ++q) {
            vector<ll> full = complete_turn_values(terminal);

            ll otherCost = 0;
            for (int p = 1; p < K; ++p) {
                otherCost += full[X[p]];
            }

            WalkResult next = walk_from_stops(playerPosition);
            for (int v = 0; v < N; ++v) {
                if (next.reach[v] < INF / 4)
                    answer[v] = min(answer[v], next.reach[v] + otherCost);
            }

            for (int s : stops) terminal[s] = full[s];
            playerPosition.swap(next.arrival);
        }

        for (ll value : answer) cout << value << '\n';
        return 0;
    }

    /*
       Parametric preparation.

       first[v] is the minimum cost of completing one turn from v.
    */
    vector<ll> zeroTerminal(N, INF);
    for (int s : stops) zeroTerminal[s] = 0;
    vector<ll> first = complete_turn_values(zeroTerminal);

    /*
       A good stop is incident to a stop-stop edge.  h[v] is the minimum
       (number of moves - number of completed turns) needed to reach one.
    */
    vector<char> good(N, false);
    for (auto [u, v] : edges) {
        if (S[u] == '1' && S[v] == '1') {
            good[u] = good[v] = true;
        }
    }

    vector<ll> h(N, INF);
    deque<int> dq;
    for (int s : stops) {
        if (good[s]) {
            h[s] = 0;
            dq.push_back(s);
        }
    }

    while (!dq.empty()) {
        int v = dq.front();
        dq.pop_front();

        int w = (S[v] == '1' ? 0 : 1);
        for (int u : graph[v]) {
            if (h[v] + w < h[u]) {
                h[u] = h[v] + w;
                if (w == 0) dq.push_front(u);
                else dq.push_back(u);
            }
        }
    }

    int P = K - 1;
    ll intercept = 0;
    vector<pair<ll,ll>> switches;
    switches.reserve(P);

    for (int p = 1; p < K; ++p) {
        ll a = first[X[p]] - 2;
        intercept += a;

        if (h[X[p]] < INF / 4) {
            ll threshold = h[X[p]] - a;
            switches.push_back({threshold, h[X[p]] - a});
        }
    }

    sort(switches.begin(), switches.end());

    vector<pair<ll,ll>> lines;
    ll slope = 2LL * P;
    lines.push_back({slope, intercept});

    for (int i = 0; i < (int)switches.size(); ) {
        int j = i;
        ll delta = 0;
        while (j < (int)switches.size() &&
               switches[j].first == switches[i].first) {
            delta += switches[j].second;
            ++j;
        }

        slope -= (j - i);
        intercept += delta;
        lines.push_back({slope, intercept});
        i = j;
    }

    /*
       All affine pieces are exact when their number is manageable.
       Otherwise retain only the always-feasible two-move strategy.
    */
    if ((int)lines.size() > 120) {
        lines.resize(1);
    }

    auto run_parametric = [&](ll lambda, ll constant) {
        int states = 3 * N;
        vector<ll> dist(states, INF);

        auto id = [&](int v, int count) {
            return count * N + v;
        };

        priority_queue<pair<ll,int>,
                       vector<pair<ll,int>>,
                       greater<pair<ll,int>>> pq;

        dist[id(X[0], 0)] = 0;
        pq.push({0, id(X[0], 0)});

        while (!pq.empty()) {
            auto [du, state] = pq.top();
            pq.pop();
            if (du != dist[state]) continue;

            int count = state / N;
            int u = state % N;

            for (int v : graph[u]) {
                int nextCount = count;
                ll weight = 1;

                if (S[v] == '1') {
                    nextCount = min(2, count + 1);
                    weight += lambda;
                }

                int ns = id(v, nextCount);
                if (du + weight < dist[ns]) {
                    dist[ns] = du + weight;
                    pq.push({dist[ns], ns});
                }
            }
        }

        for (int v = 0; v < N; ++v) {
            ll candidate = INF;

            if (S[v] == '0') {
                candidate = min(dist[id(v, 1)], dist[id(v, 2)]);
            } else {
                candidate = dist[id(v, 2)];
                if (candidate < INF / 4) candidate -= lambda;
            }

            if (candidate < INF / 4)
                answer[v] = min(answer[v], candidate + constant);
        }
    };

    for (auto [lambda, constant] : lines) {
        run_parametric(lambda, constant);
    }

    for (ll value : answer) cout << value << '\n';
    return 0;
}
