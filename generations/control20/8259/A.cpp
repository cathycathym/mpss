#include <bits/stdc++.h>
using namespace std;

struct DSU {
    vector<int> p, sz;
    DSU(int n) : p(n), sz(n, 1) {
        iota(p.begin(), p.end(), 0);
    }
    int find(int x) {
        while (p[x] != x) {
            p[x] = p[p[x]];
            x = p[x];
        }
        return x;
    }
    void unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return;
        if (sz[a] < sz[b]) swap(a, b);
        p[b] = a;
        sz[a] += sz[b];
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int H, W, Q;
    cin >> H >> W >> Q;
    const int N = H * W;

    DSU dsu(N);
    string s;

    for (int i = 0; i < H; ++i) {
        cin >> s;
        for (int j = 0; j + 1 < W; ++j) {
            if (s[j] == '1')
                dsu.unite(i * W + j, i * W + j + 1);
        }
    }

    for (int i = 0; i + 1 < H; ++i) {
        cin >> s;
        for (int j = 0; j < W; ++j) {
            if (s[j] == '1')
                dsu.unite(i * W + j, (i + 1) * W + j);
        }
    }

    vector<int> C(H);
    bool allSame = true;
    for (int i = 0; i < H; ++i) {
        cin >> C[i];
        if (i && C[i] != C[0]) allSame = false;
    }

    vector<int> rootId(N, -1), cellComp(N);
    vector<int> top, bottom;
    int M = 0;

    for (int i = 0; i < H; ++i) {
        for (int j = 0; j < W; ++j) {
            int v = i * W + j;
            int r = dsu.find(v);
            if (rootId[r] == -1) {
                rootId[r] = M++;
                top.push_back(i);
                bottom.push_back(i);
            }
            int id = rootId[r];
            cellComp[v] = id;
            top[id] = min(top[id], i);
            bottom[id] = max(bottom[id], i);
        }
    }

    vector<vector<int>> queries(Q);
    bool everyPair = true;
    for (int qi = 0; qi < Q; ++qi) {
        int T;
        cin >> T;
        if (T != 2) everyPair = false;
        queries[qi].reserve(T);
        for (int k = 0; k < T; ++k) {
            int x, y;
            cin >> x >> y;
            --x; --y;
            queries[qi].push_back(cellComp[x * W + y]);
        }
        sort(queries[qi].begin(), queries[qi].end());
        queries[qi].erase(unique(queries[qi].begin(), queries[qi].end()),
                          queries[qi].end());
    }

    /*
       When all repair costs are equal, reachability after a fixed number of
       repairs is represented by an interval of rows.  If [L,R] is currently
       reachable, one further repair expands it to
           [ minTopOfComponentContaining(L),
             maxBottomOfComponentContaining(R) ].
       Binary lifting therefore answers every two-terminal query quickly.
    */
    if (allSame && everyPair) {
        vector<int> headTop(H, -1), headBottom(H, -1);
        vector<int> nextTop(M, -1), nextBottom(M, -1);

        for (int id = 0; id < M; ++id) {
            nextTop[id] = headTop[top[id]];
            headTop[top[id]] = id;
            nextBottom[id] = headBottom[bottom[id]];
            headBottom[bottom[id]] = id;
        }

        vector<int> goRight(H), goLeft(H);

        int farthest = -1;
        for (int row = 0; row < H; ++row) {
            for (int id = headTop[row]; id != -1; id = nextTop[id])
                farthest = max(farthest, bottom[id]);
            goRight[row] = farthest;
        }

        int earliest = H;
        for (int row = H - 1; row >= 0; --row) {
            for (int id = headBottom[row]; id != -1; id = nextBottom[id])
                earliest = min(earliest, top[id]);
            goLeft[row] = earliest;
        }

        int LOG = 1;
        while ((1 << LOG) <= H + 1) ++LOG;

        vector<vector<int>> upR(LOG, vector<int>(H));
        vector<vector<int>> upL(LOG, vector<int>(H));
        upR[0] = goRight;
        upL[0] = goLeft;

        for (int k = 1; k < LOG; ++k) {
            for (int i = 0; i < H; ++i) {
                upR[k][i] = upR[k - 1][upR[k - 1][i]];
                upL[k][i] = upL[k - 1][upL[k - 1][i]];
            }
        }

        for (const auto &q : queries) {
            if (q.size() == 1) {
                cout << 0 << '\n';
                continue;
            }

            int a = q[0], b = q[1];
            if (top[a] > top[b]) swap(a, b);

            if (max(top[a], top[b]) <= min(bottom[a], bottom[b])) {
                cout << C[0] << '\n';
                continue;
            }

            long long expansions = 0;
            bool possible = true;

            if (bottom[a] < top[b]) {
                int cur = bottom[a];
                int target = top[b];

                if (goRight[cur] == cur) {
                    possible = false;
                } else {
                    for (int k = LOG - 1; k >= 0; --k) {
                        int nxt = upR[k][cur];
                        if (nxt < target && nxt > cur) {
                            cur = nxt;
                            expansions += 1LL << k;
                        }
                    }
                    if (goRight[cur] < target)
                        possible = false;
                    else
                        ++expansions;
                }
            } else {
                int cur = top[a];
                int target = bottom[b];

                if (goLeft[cur] == cur) {
                    possible = false;
                } else {
                    for (int k = LOG - 1; k >= 0; --k) {
                        int nxt = upL[k][cur];
                        if (nxt > target && nxt < cur) {
                            cur = nxt;
                            expansions += 1LL << k;
                        }
                    }
                    if (goLeft[cur] > target)
                        possible = false;
                    else
                        ++expansions;
                }
            }

            if (!possible)
                cout << -1 << '\n';
            else
                cout << (expansions + 1) * C[0] << '\n';
        }
        return 0;
    }

    /*
       Contracting the initially passable road network gives component nodes.
       Repairing a row connects every component occurring in that row.  The
       following CSR representation is the resulting component-row incidence
       graph.  Entering an unrepaired row costs C[row].
    */
    vector<int> rowDeg(H, 0), compDeg(M, 0), seen(M, -1);
    int E = 0;

    for (int i = 0; i < H; ++i) {
        for (int j = 0; j < W; ++j) {
            int id = cellComp[i * W + j];
            if (seen[id] != i) {
                seen[id] = i;
                ++rowDeg[i];
                ++compDeg[id];
                ++E;
            }
        }
    }

    vector<int> rowOff(H + 1, 0), compOff(M + 1, 0);
    for (int i = 0; i < H; ++i) rowOff[i + 1] = rowOff[i] + rowDeg[i];
    for (int i = 0; i < M; ++i) compOff[i + 1] = compOff[i] + compDeg[i];

    vector<int> rowAdj(E), compAdj(E);
    vector<int> rowCur = rowOff, compCur = compOff;
    fill(seen.begin(), seen.end(), -1);

    for (int i = 0; i < H; ++i) {
        for (int j = 0; j < W; ++j) {
            int id = cellComp[i * W + j];
            if (seen[id] != i) {
                seen[id] = i;
                rowAdj[rowCur[i]++] = id;
                compAdj[compCur[id]++] = i;
            }
        }
    }

    const int INF = 1e9;
    const int V = M + H;

    auto shortestPair = [&](int source, int target) -> int {
        if (source == target) return 0;

        vector<int> dist(V, INF);
        priority_queue<pair<int,int>,
                       vector<pair<int,int>>,
                       greater<pair<int,int>>> pq;

        dist[source] = 0;
        pq.push({0, source});

        while (!pq.empty()) {
            auto [d, v] = pq.top();
            pq.pop();
            if (d != dist[v]) continue;
            if (v == target) return d;

            if (v < M) {
                for (int e = compOff[v]; e < compOff[v + 1]; ++e) {
                    int row = compAdj[e];
                    int to = M + row;
                    int nd = d + C[row];
                    if (nd < dist[to]) {
                        dist[to] = nd;
                        pq.push({nd, to});
                    }
                }
            } else {
                int row = v - M;
                for (int e = rowOff[row]; e < rowOff[row + 1]; ++e) {
                    int to = rowAdj[e];
                    if (d < dist[to]) {
                        dist[to] = d;
                        pq.push({d, to});
                    }
                }
            }
        }
        return -1;
    };

    auto connectMany = [&](const vector<int> &terminals) -> int {
        if (terminals.size() <= 1) return 0;
        if (terminals.size() == 2)
            return shortestPair(terminals[0], terminals[1]);

        vector<char> connected(M, 0), activated(H, 0);
        vector<int> parent(V), dist(V);
        connected[terminals[0]] = 1;
        int connectedTerminals = 1;
        long long answer = 0;

        while (connectedTerminals < (int)terminals.size()) {
            fill(dist.begin(), dist.end(), INF);
            fill(parent.begin(), parent.end(), -1);

            priority_queue<pair<int,int>,
                           vector<pair<int,int>>,
                           greater<pair<int,int>>> pq;

            for (int id = 0; id < M; ++id) {
                if (connected[id]) {
                    dist[id] = 0;
                    pq.push({0, id});
                }
            }

            int reached = -1;
            while (!pq.empty()) {
                auto [d, v] = pq.top();
                pq.pop();
                if (d != dist[v]) continue;

                if (v < M && !connected[v] &&
                    binary_search(terminals.begin(), terminals.end(), v)) {
                    reached = v;
                    break;
                }

                if (v < M) {
                    for (int e = compOff[v]; e < compOff[v + 1]; ++e) {
                        int row = compAdj[e];
                        int to = M + row;
                        int nd = d + (activated[row] ? 0 : C[row]);
                        if (nd < dist[to]) {
                            dist[to] = nd;
                            parent[to] = v;
                            pq.push({nd, to});
                        }
                    }
                } else {
                    int row = v - M;
                    for (int e = rowOff[row]; e < rowOff[row + 1]; ++e) {
                        int to = rowAdj[e];
                        if (d < dist[to]) {
                            dist[to] = d;
                            parent[to] = v;
                            pq.push({d, to});
                        }
                    }
                }
            }

            if (reached == -1) return -1;

            int v = reached;
            while (v != -1) {
                if (v < M) {
                    connected[v] = 1;
                } else {
                    int row = v - M;
                    if (!activated[row]) {
                        activated[row] = 1;
                        answer += C[row];
                    }
                }
                v = parent[v];
            }

            queue<int> cq, rq;
            vector<char> rowSeen(H, 0);
            for (int id = 0; id < M; ++id)
                if (connected[id]) cq.push(id);

            while (!cq.empty() || !rq.empty()) {
                while (!cq.empty()) {
                    int id = cq.front();
                    cq.pop();
                    for (int e = compOff[id]; e < compOff[id + 1]; ++e) {
                        int row = compAdj[e];
                        if (activated[row] && !rowSeen[row]) {
                            rowSeen[row] = 1;
                            rq.push(row);
                        }
                    }
                }
                while (!rq.empty()) {
                    int row = rq.front();
                    rq.pop();
                    for (int e = rowOff[row]; e < rowOff[row + 1]; ++e) {
                        int id = rowAdj[e];
                        if (!connected[id]) {
                            connected[id] = 1;
                            cq.push(id);
                        }
                    }
                }
            }

            connectedTerminals = 0;
            for (int id : terminals)
                if (connected[id]) ++connectedTerminals;
        }

        return answer > INF ? -1 : (int)answer;
    };

    for (auto q : queries) {
        sort(q.begin(), q.end());
        q.erase(unique(q.begin(), q.end()), q.end());
        cout << connectMany(q) << '\n';
    }

    return 0;
}
