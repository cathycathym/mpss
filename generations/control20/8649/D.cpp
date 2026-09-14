#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
const int64 INF = (1LL << 62);

struct Flight {
    int a, b;
    int parent = -1;
    int firstChild = -1;
    int nextSibling = -1;
    int tin = -1;
    int tout = -1;
    int64 edgeWeight = 0;
    int64 potential = 0;
};

struct Layer {
    vector<int> ids;
    vector<int> tins;
    vector<vector<int64>> sparse;
    int64 singleAnswer = INF;
};

struct CustomHash {
    static uint64_t splitmix64(uint64_t x) {
        x += 0x9e3779b97f4a7c15ULL;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
        x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
        return x ^ (x >> 31);
    }

    size_t operator()(uint64_t x) const {
        static const uint64_t seed =
            chrono::steady_clock::now().time_since_epoch().count();
        return splitmix64(x + seed);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    int64 T;
    cin >> N >> T;

    const int E = N - 1;
    vector<Layer> layers(E);
    vector<Flight> flights;
    flights.reserve(100000);

    for (int i = 0; i < E; ++i) {
        int m;
        cin >> m;
        layers[i].ids.reserve(m);

        for (int j = 0; j < m; ++j) {
            int a, b;
            cin >> a >> b;

            int id = static_cast<int>(flights.size());
            flights.push_back(Flight());
            flights.back().a = a;
            flights.back().b = b;
            layers[i].ids.push_back(id);
            layers[i].singleAnswer =
                min(layers[i].singleAnswer, int64(b) - a);
        }

        sort(layers[i].ids.begin(), layers[i].ids.end(),
             [&](int x, int y) {
                 if (flights[x].a != flights[y].a)
                     return flights[x].a < flights[y].a;
                 return flights[x].b < flights[y].b;
             });
    }

    for (int i = 0; i + 1 < E; ++i) {
        const vector<int>& nextIds = layers[i + 1].ids;
        int m = static_cast<int>(nextIds.size());

        vector<int> suffixBest(m);
        suffixBest[m - 1] = nextIds[m - 1];

        for (int j = m - 2; j >= 0; --j) {
            int x = nextIds[j];
            int y = suffixBest[j + 1];
            suffixBest[j] = (flights[x].b <= flights[y].b ? x : y);
        }

        for (int u : layers[i].ids) {
            int arrival = flights[u].b;

            int pos = static_cast<int>(
                lower_bound(
                    nextIds.begin(), nextIds.end(), arrival,
                    [&](int id, int value) {
                        return flights[id].a < value;
                    }
                ) - nextIds.begin()
            );

            int v;
            int64 weight;

            if (pos < m) {
                v = suffixBest[pos];
                weight = 0;
            } else {
                v = suffixBest[0];
                weight = T;
            }

            flights[u].parent = v;
            flights[u].edgeWeight = weight;
            flights[u].nextSibling = flights[v].firstChild;
            flights[v].firstChild = u;
        }
    }

    for (int i = E - 2; i >= 0; --i) {
        for (int u : layers[i].ids) {
            int v = flights[u].parent;
            flights[u].potential =
                flights[v].potential + flights[u].edgeWeight;
        }
    }

    int timer = 0;
    vector<pair<int, bool>> dfsStack;
    dfsStack.reserve(2 * flights.size());

    for (int root : layers[E - 1].ids) {
        dfsStack.push_back(make_pair(root, false));

        while (!dfsStack.empty()) {
            int u = dfsStack.back().first;
            bool exiting = dfsStack.back().second;
            dfsStack.pop_back();

            if (!exiting) {
                flights[u].tin = timer++;
                dfsStack.push_back(make_pair(u, true));

                for (int child = flights[u].firstChild;
                     child != -1;
                     child = flights[child].nextSibling) {
                    dfsStack.push_back(make_pair(child, false));
                }
            } else {
                flights[u].tout = timer - 1;
            }
        }
    }

    int maxLayerSize = 1;
    for (const Layer& layer : layers) {
        maxLayerSize =
            max(maxLayerSize, static_cast<int>(layer.ids.size()));
    }

    vector<int> floorLog(maxLayerSize + 1, 0);
    for (int i = 2; i <= maxLayerSize; ++i)
        floorLog[i] = floorLog[i / 2] + 1;

    for (Layer& layer : layers) {
        sort(layer.ids.begin(), layer.ids.end(),
             [&](int x, int y) {
                 return flights[x].tin < flights[y].tin;
             });

        int m = static_cast<int>(layer.ids.size());
        layer.tins.resize(m);

        for (int i = 0; i < m; ++i)
            layer.tins[i] = flights[layer.ids[i]].tin;

        int levels = floorLog[m] + 1;
        layer.sparse.resize(levels);
        layer.sparse[0].resize(m);

        for (int i = 0; i < m; ++i) {
            int u = layer.ids[i];
            layer.sparse[0][i] =
                flights[u].potential - flights[u].a;
        }

        for (int k = 1; k < levels; ++k) {
            int len = 1 << k;
            int half = len >> 1;
            int count = m - len + 1;
            layer.sparse[k].resize(max(0, count));

            for (int i = 0; i < count; ++i) {
                layer.sparse[k][i] =
                    min(layer.sparse[k - 1][i],
                        layer.sparse[k - 1][i + half]);
            }
        }
    }

    auto rangeMinimum = [&](const Layer& layer,
                            int left, int right) -> int64 {
        int len = right - left;
        int k = floorLog[len];
        int block = 1 << k;
        return min(layer.sparse[k][left],
                   layer.sparse[k][right - block]);
    };

    auto solvePair = [&](int startLayer, int finalLayer) -> int64 {
        if (startLayer == finalLayer)
            return layers[startLayer].singleAnswer;

        const Layer& start = layers[startLayer];
        const Layer& finish = layers[finalLayer];

        int ms = static_cast<int>(start.ids.size());
        int mf = static_cast<int>(finish.ids.size());

        int logStart = floorLog[ms] + 1;
        int logFinish = floorLog[mf] + 1;

        int64 costFromStart = int64(ms) * logFinish;
        int64 costFromFinish = int64(mf) * (2 * logStart + 1);
        int64 costMerge = int64(ms) + mf;

        int64 answer = INF;

        if (costMerge <= costFromStart &&
            costMerge <= costFromFinish) {
            int p = 0;

            for (int v : finish.ids) {
                while (p < ms &&
                       flights[start.ids[p]].tin < flights[v].tin) {
                    ++p;
                }

                while (p < ms &&
                       flights[start.ids[p]].tin <= flights[v].tout) {
                    int u = start.ids[p];
                    answer = min(
                        answer,
                        flights[u].potential
                            - flights[v].potential
                            + int64(flights[v].b)
                            - flights[u].a
                    );
                    ++p;
                }
            }
        } else if (costFromStart <= costFromFinish) {
            for (int u : start.ids) {
                int pos = static_cast<int>(
                    upper_bound(
                        finish.tins.begin(),
                        finish.tins.end(),
                        flights[u].tin
                    ) - finish.tins.begin()
                ) - 1;

                if (pos < 0)
                    continue;

                int v = finish.ids[pos];
                if (flights[u].tin > flights[v].tout)
                    continue;

                answer = min(
                    answer,
                    flights[u].potential
                        - flights[v].potential
                        + int64(flights[v].b)
                        - flights[u].a
                );
            }
        } else {
            for (int v : finish.ids) {
                int left = static_cast<int>(
                    lower_bound(
                        start.tins.begin(),
                        start.tins.end(),
                        flights[v].tin
                    ) - start.tins.begin()
                );

                int right = static_cast<int>(
                    upper_bound(
                        start.tins.begin(),
                        start.tins.end(),
                        flights[v].tout
                    ) - start.tins.begin()
                );

                if (left == right)
                    continue;

                int64 bestStart =
                    rangeMinimum(start, left, right);

                answer = min(
                    answer,
                    bestStart
                        - flights[v].potential
                        + flights[v].b
                );
            }
        }

        return answer;
    };

    int Q;
    cin >> Q;

    unordered_map<uint64_t, int64, CustomHash> memo;
    memo.reserve(static_cast<size_t>(Q) * 2 + 1);
    memo.max_load_factor(0.7f);

    vector<int64> answers;
    answers.reserve(Q);

    for (int qi = 0; qi < Q; ++qi) {
        int L, R;
        cin >> L >> R;

        int startLayer = L - 1;
        int finalLayer = R - 2;

        uint64_t key =
            (uint64_t(uint32_t(startLayer)) << 32)
            | uint32_t(finalLayer);

        unordered_map<uint64_t, int64, CustomHash>::iterator it =
            memo.find(key);

        if (it != memo.end()) {
            answers.push_back(it->second);
        } else {
            int64 result = solvePair(startLayer, finalLayer);
            memo.emplace(key, result);
            answers.push_back(result);
        }
    }

    for (int64 answer : answers)
        cout << answer << '\n';

    return 0;
}
