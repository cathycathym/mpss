#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
const int64 INF = (1LL << 62);

struct Edge {
    int offset = 0;
    vector<int> a, b;
    vector<int> orderA, orderB;
    vector<int> sortedA, sortedB;
};

struct Query {
    int l, e;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    int64 T;
    cin >> N >> T;

    const int E = N - 1;
    vector<Edge> edges(E);
    int totalFlights = 0;

    for (int i = 0; i < E; ++i) {
        int m;
        cin >> m;
        edges[i].offset = totalFlights;
        edges[i].a.resize(m);
        edges[i].b.resize(m);

        for (int j = 0; j < m; ++j) {
            cin >> edges[i].a[j] >> edges[i].b[j];
        }

        edges[i].orderA.resize(m);
        edges[i].orderB.resize(m);
        iota(edges[i].orderA.begin(), edges[i].orderA.end(), 0);
        iota(edges[i].orderB.begin(), edges[i].orderB.end(), 0);

        sort(edges[i].orderA.begin(), edges[i].orderA.end(),
             [&](int x, int y) {
                 if (edges[i].a[x] != edges[i].a[y])
                     return edges[i].a[x] < edges[i].a[y];
                 return edges[i].b[x] < edges[i].b[y];
             });

        sort(edges[i].orderB.begin(), edges[i].orderB.end(),
             [&](int x, int y) {
                 if (edges[i].b[x] != edges[i].b[y])
                     return edges[i].b[x] < edges[i].b[y];
                 return edges[i].a[x] < edges[i].a[y];
             });

        edges[i].sortedA.resize(m);
        edges[i].sortedB.resize(m);
        for (int j = 0; j < m; ++j) {
            edges[i].sortedA[j] = edges[i].a[edges[i].orderA[j]];
            edges[i].sortedB[j] = edges[i].b[edges[i].orderB[j]];
        }

        totalFlights += m;
    }

    vector<int> globalA(totalFlights), globalB(totalFlights);
    for (const Edge &edge : edges) {
        for (int j = 0; j < (int)edge.a.size(); ++j) {
            globalA[edge.offset + j] = edge.a[j];
            globalB[edge.offset + j] = edge.b[j];
        }
    }

    int LOG = 1;
    while ((1 << LOG) <= E) ++LOG;

    vector<vector<int>> jumpForward(LOG, vector<int>(totalFlights, -1));
    vector<vector<int>> daysForward(LOG, vector<int>(totalFlights, 0));
    vector<vector<int>> jumpBackward(LOG, vector<int>(totalFlights, -1));
    vector<vector<int>> daysBackward(LOG, vector<int>(totalFlights, 0));

    for (int i = 0; i + 1 < E; ++i) {
        const Edge &from = edges[i];
        const Edge &to = edges[i + 1];
        int m = (int)to.a.size();

        vector<int> suffixBest(m);
        int best = -1;
        for (int p = m - 1; p >= 0; --p) {
            int local = to.orderA[p];
            if (best == -1 || to.b[local] < to.b[best])
                best = local;
            suffixBest[p] = best;
        }

        int minimumArrival = 0;
        for (int j = 1; j < m; ++j) {
            if (to.b[j] < to.b[minimumArrival])
                minimumArrival = j;
        }

        for (int p = 0; p < (int)from.a.size(); ++p) {
            int pos = lower_bound(to.sortedA.begin(), to.sortedA.end(),
                                  from.b[p]) - to.sortedA.begin();
            int id = from.offset + p;
            if (pos < m) {
                jumpForward[0][id] = to.offset + suffixBest[pos];
                daysForward[0][id] = 0;
            } else {
                jumpForward[0][id] = to.offset + minimumArrival;
                daysForward[0][id] = 1;
            }
        }
    }

    for (int i = 1; i < E; ++i) {
        const Edge &from = edges[i - 1];
        const Edge &to = edges[i];
        int m = (int)from.a.size();

        vector<int> prefixBest(m);
        int best = -1;
        for (int p = 0; p < m; ++p) {
            int local = from.orderB[p];
            if (best == -1 || from.a[local] > from.a[best])
                best = local;
            prefixBest[p] = best;
        }

        int maximumDeparture = 0;
        for (int j = 1; j < m; ++j) {
            if (from.a[j] > from.a[maximumDeparture])
                maximumDeparture = j;
        }

        for (int q = 0; q < (int)to.a.size(); ++q) {
            int count = upper_bound(from.sortedB.begin(), from.sortedB.end(),
                                    to.a[q]) - from.sortedB.begin();
            int id = to.offset + q;
            if (count > 0) {
                jumpBackward[0][id] = from.offset + prefixBest[count - 1];
                daysBackward[0][id] = 0;
            } else {
                jumpBackward[0][id] = from.offset + maximumDeparture;
                daysBackward[0][id] = 1;
            }
        }
    }

    for (int k = 1; k < LOG; ++k) {
        for (int id = 0; id < totalFlights; ++id) {
            int mid = jumpForward[k - 1][id];
            if (mid != -1 && jumpForward[k - 1][mid] != -1) {
                jumpForward[k][id] = jumpForward[k - 1][mid];
                daysForward[k][id] =
                    daysForward[k - 1][id] + daysForward[k - 1][mid];
            }

            mid = jumpBackward[k - 1][id];
            if (mid != -1 && jumpBackward[k - 1][mid] != -1) {
                jumpBackward[k][id] = jumpBackward[k - 1][mid];
                daysBackward[k][id] =
                    daysBackward[k - 1][id] + daysBackward[k - 1][mid];
            }
        }
    }

    int Q;
    cin >> Q;
    vector<Query> queries(Q);
    for (int i = 0; i < Q; ++i) {
        int l, r;
        cin >> l >> r;
        --l;
        queries[i] = {l, r - 2};
    }

    vector<int64> edgePrefix(E + 1, 0);
    for (int i = 0; i < E; ++i)
        edgePrefix[i + 1] = edgePrefix[i] + edges[i].a.size();

    vector<int> thresholdCandidates = {
        4, 8, 12, 16, 24, 32, 48, 64,
        80, 96, 128, 192, 256, 384, 512, 768, 1024
    };

    int threshold = 64;
    bool useForwardSweeps = true;
    int64 bestEstimate = INF;

    vector<int> maxEnd(E), minStart(E);

    for (int candidate : thresholdCandidates) {
        fill(maxEnd.begin(), maxEnd.end(), -1);
        fill(minStart.begin(), minStart.end(), E);

        int64 lightCost = 0;
        for (const Query &query : queries) {
            int leftSize = edges[query.l].a.size();
            int rightSize = edges[query.e].a.size();

            if (leftSize <= candidate || rightSize <= candidate) {
                int count = min(leftSize, rightSize);
                int distance = query.e - query.l;
                lightCost += 1LL * count *
                             max(1, __builtin_popcount((unsigned)distance));
            } else {
                maxEnd[query.l] = max(maxEnd[query.l], query.e);
                minStart[query.e] = min(minStart[query.e], query.l);
            }
        }

        int64 forwardCost = 0;
        int64 backwardCost = 0;

        for (int l = 0; l < E; ++l) {
            if (maxEnd[l] != -1)
                forwardCost += edgePrefix[maxEnd[l] + 1] - edgePrefix[l];
        }
        for (int e = 0; e < E; ++e) {
            if (minStart[e] != E)
                backwardCost += edgePrefix[e + 1] - edgePrefix[minStart[e]];
        }

        int64 estimate = lightCost + min(forwardCost, backwardCost);
        if (estimate < bestEstimate) {
            bestEstimate = estimate;
            threshold = candidate;
            useForwardSweeps = (forwardCost <= backwardCost);
        }
    }

    vector<int64> answer(Q, INF);
    vector<vector<pair<int, int>>> grouped(E);

    auto solveForwardCandidate = [&](int startId, int distance) -> int64 {
        int current = startId;
        int days = 0;
        for (int k = 0; k < LOG; ++k) {
            if ((distance >> k) & 1) {
                days += daysForward[k][current];
                current = jumpForward[k][current];
            }
        }
        return 1LL * days * T + globalB[current] - globalA[startId];
    };

    auto solveBackwardCandidate = [&](int endId, int distance) -> int64 {
        int current = endId;
        int days = 0;
        for (int k = 0; k < LOG; ++k) {
            if ((distance >> k) & 1) {
                days += daysBackward[k][current];
                current = jumpBackward[k][current];
            }
        }
        return 1LL * days * T + globalB[endId] - globalA[current];
    };

    for (int qi = 0; qi < Q; ++qi) {
        int l = queries[qi].l;
        int e = queries[qi].e;
        int leftSize = edges[l].a.size();
        int rightSize = edges[e].a.size();
        int distance = e - l;

        if (leftSize <= threshold || rightSize <= threshold) {
            if (leftSize <= rightSize) {
                for (int p = 0; p < leftSize; ++p) {
                    int id = edges[l].offset + p;
                    answer[qi] = min(answer[qi],
                                     solveForwardCandidate(id, distance));
                }
            } else {
                for (int p = 0; p < rightSize; ++p) {
                    int id = edges[e].offset + p;
                    answer[qi] = min(answer[qi],
                                     solveBackwardCandidate(id, distance));
                }
            }
        } else if (useForwardSweeps) {
            grouped[l].push_back({e, qi});
        } else {
            grouped[e].push_back({l, qi});
        }
    }

    if (useForwardSweeps) {
        vector<int64> dp, nextDp, suffixMin;

        for (int start = 0; start < E; ++start) {
            if (grouped[start].empty()) continue;

            sort(grouped[start].begin(), grouped[start].end());
            int farthest = grouped[start].back().first;
            size_t requestPos = 0;

            dp.resize(edges[start].a.size());
            for (int p = 0; p < (int)dp.size(); ++p)
                dp[p] = -1LL * edges[start].a[p];

            for (int current = start; current <= farthest; ++current) {
                if (requestPos < grouped[start].size() &&
                    grouped[start][requestPos].first == current) {
                    int64 best = INF;
                    for (int p = 0; p < (int)dp.size(); ++p)
                        best = min(best, dp[p] + edges[current].b[p]);

                    while (requestPos < grouped[start].size() &&
                           grouped[start][requestPos].first == current) {
                        answer[grouped[start][requestPos].second] = best;
                        ++requestPos;
                    }
                }

                if (current == farthest) break;

                const Edge &from = edges[current];
                const Edge &to = edges[current + 1];
                int m = from.a.size();

                suffixMin.resize(m + 1);
                suffixMin[m] = INF;
                for (int pos = m - 1; pos >= 0; --pos) {
                    int local = from.orderB[pos];
                    suffixMin[pos] = min(suffixMin[pos + 1], dp[local]);
                }

                nextDp.resize(to.a.size());
                int pos = 0;
                int64 lowMinimum = INF;

                for (int localTo : to.orderA) {
                    int departure = to.a[localTo];
                    while (pos < m &&
                           from.b[from.orderB[pos]] <= departure) {
                        lowMinimum =
                            min(lowMinimum, dp[from.orderB[pos]]);
                        ++pos;
                    }
                    nextDp[localTo] =
                        min(lowMinimum, T + suffixMin[pos]);
                }

                dp.swap(nextDp);
            }
        }
    } else {
        vector<int64> dp, previousDp, prefixMin, suffixMin;

        for (int finish = 0; finish < E; ++finish) {
            if (grouped[finish].empty()) continue;

            sort(grouped[finish].begin(), grouped[finish].end(),
                 greater<pair<int, int>>());
            int earliest = grouped[finish].back().first;
            for (const auto &entry : grouped[finish])
                earliest = min(earliest, entry.first);

            size_t requestPos = 0;
            dp.resize(edges[finish].a.size());
            for (int q = 0; q < (int)dp.size(); ++q)
                dp[q] = edges[finish].b[q];

            for (int current = finish; current >= earliest; --current) {
                while (requestPos < grouped[finish].size() &&
                       grouped[finish][requestPos].first > current) {
                    ++requestPos;
                }

                if (requestPos < grouped[finish].size() &&
                    grouped[finish][requestPos].first == current) {
                    int64 best = INF;
                    for (int p = 0; p < (int)dp.size(); ++p)
                        best = min(best, dp[p] - edges[current].a[p]);

                    while (requestPos < grouped[finish].size() &&
                           grouped[finish][requestPos].first == current) {
                        answer[grouped[finish][requestPos].second] = best;
                        ++requestPos;
                    }
                }

                if (current == earliest) break;

                const Edge &to = edges[current];
                const Edge &from = edges[current - 1];
                int m = to.a.size();

                prefixMin.resize(m);
                suffixMin.resize(m + 1);

                int64 value = INF;
                for (int pos = 0; pos < m; ++pos) {
                    value = min(value, dp[to.orderA[pos]]);
                    prefixMin[pos] = value;
                }

                suffixMin[m] = INF;
                for (int pos = m - 1; pos >= 0; --pos) {
                    suffixMin[pos] =
                        min(suffixMin[pos + 1], dp[to.orderA[pos]]);
                }

                previousDp.resize(from.a.size());
                int pos = 0;

                for (int localFrom : from.orderB) {
                    int arrival = from.b[localFrom];
                    while (pos < m &&
                           to.a[to.orderA[pos]] < arrival) {
                        ++pos;
                    }

                    int64 sameDay = suffixMin[pos];
                    int64 nextDay =
                        (pos == 0 ? INF : T + prefixMin[pos - 1]);
                    previousDp[localFrom] = min(sameDay, nextDay);
                }

                dp.swap(previousDp);
            }
        }
    }

    for (int i = 0; i < Q; ++i)
        cout << answer[i] << '\n';

    return 0;
}
