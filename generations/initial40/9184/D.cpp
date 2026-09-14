#include <bits/stdc++.h>
using namespace std;

struct Bag {
    unordered_map<int, int> count;
    int value = 0;

    Bag() {
        count.max_load_factor(0.7f);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, K;
    cin >> N >> K;

    vector<int> language(N);
    for (int &x : language) cin >> x;

    vector<vector<int>> children(N);
    for (int v = 1; v < N; ++v) {
        int boss;
        cin >> boss;
        children[boss].push_back(v);
    }

    vector<int> depth(N), tin(N), tout(N), preorder;
    vector<vector<int>> nodesAtDepth;
    vector<vector<int>> languageEuler(K);

    preorder.reserve(N);

    int timer = 0;
    vector<pair<int, int>> stack;
    stack.push_back({0, 0});

    while (!stack.empty()) {
        int v = stack.back().first;
        int &nextChild = stack.back().second;

        if (nextChild == 0) {
            tin[v] = timer++;
            preorder.push_back(v);

            if ((int)nodesAtDepth.size() <= depth[v])
                nodesAtDepth.resize(depth[v] + 1);

            nodesAtDepth[depth[v]].push_back(tin[v]);
            languageEuler[language[v]].push_back(tin[v]);
        }

        if (nextChild < (int)children[v].size()) {
            int u = children[v][nextChild++];
            depth[u] = depth[v] + 1;
            stack.push_back({u, 0});
        } else {
            tout[v] = timer;
            stack.pop_back();
        }
    }

    vector<vector<pair<int, int>>> languageDepthCount(K);
    for (int v = 0; v < N; ++v)
        languageDepthCount[language[v]].push_back({depth[v], 1});

    for (int c = 0; c < K; ++c) {
        auto &entries = languageDepthCount[c];
        sort(entries.begin(), entries.end());

        vector<pair<int, int>> compressed;
        for (auto [d, count] : entries) {
            if (compressed.empty() || compressed.back().first != d)
                compressed.push_back({d, count});
            else
                compressed.back().second += count;
        }
        entries.swap(compressed);
    }

    auto countInInterval = [](const vector<int> &positions,
                              int left, int right) -> int {
        return int(lower_bound(positions.begin(), positions.end(), right) -
                   lower_bound(positions.begin(), positions.end(), left));
    };

    int bestPeople = 0;
    int bestSwitches = INT_MAX;

    auto consider = [&](int lead, int people) {
        int alreadyMatching =
            countInInterval(languageEuler[language[lead]],
                            tin[lead], tout[lead]);
        int switches = people - alreadyMatching;

        if (people > bestPeople ||
            (people == bestPeople && switches < bestSwitches)) {
            bestPeople = people;
            bestSwitches = switches;
        }
    };

    const int BLOCK = 700;
    vector<char> useDirectMethod(K, false);

    for (int c = 0; c < K; ++c) {
        int frequency = (int)languageEuler[c].size();
        int occupiedDepths = (int)languageDepthCount[c].size();
        useDirectMethod[c] =
            (frequency <= BLOCK || occupiedDepths <= BLOCK);
    }

    // For languages with few employees or few occupied depths, evaluate every
    // possible lead directly.
    for (int v = 0; v < N; ++v) {
        int c = language[v];
        if (!useDirectMethod[c]) continue;

        int people = 0;
        for (auto [d, available] : languageDepthCount[c]) {
            if (d < depth[v]) continue;

            int positions =
                countInInterval(nodesAtDepth[d], tin[v], tout[v]);
            people += min(positions, available);
        }
        consider(v, people);
    }

    int maxDepth = (int)nodesAtDepth.size() - 1;

    // For every remaining language, compute the answer for all its possible
    // leads using small-to-large subtree merging.
    for (int c = 0; c < K; ++c) {
        if (useDirectMethod[c]) continue;

        vector<int> capacity(maxDepth + 1, 0);
        for (auto [d, count] : languageDepthCount[c])
            capacity[d] = count;

        vector<Bag *> bag(N, nullptr);
        vector<int> answer(N, 0);

        for (int i = N - 1; i >= 0; --i) {
            int v = preorder[i];
            Bag *current = nullptr;

            for (int u : children[v]) {
                Bag *other = bag[u];
                if (other == nullptr) continue;

                if (current == nullptr) {
                    current = other;
                    continue;
                }

                if (current->count.size() < other->count.size())
                    swap(current, other);

                for (const auto &entry : other->count) {
                    int d = entry.first;
                    int added = entry.second;

                    auto it = current->count.find(d);
                    int oldCount =
                        (it == current->count.end() ? 0 : it->second);
                    int newCount = oldCount + added;

                    current->value +=
                        min(newCount, capacity[d]) -
                        min(oldCount, capacity[d]);

                    if (it == current->count.end())
                        current->count.emplace(d, newCount);
                    else
                        it->second = newCount;
                }

                delete other;
            }

            int d = depth[v];
            if (capacity[d] > 0) {
                if (current == nullptr)
                    current = new Bag();

                auto it = current->count.find(d);
                int oldCount =
                    (it == current->count.end() ? 0 : it->second);
                int newCount = oldCount + 1;

                current->value +=
                    min(newCount, capacity[d]) -
                    min(oldCount, capacity[d]);

                if (it == current->count.end())
                    current->count.emplace(d, newCount);
                else
                    it->second = newCount;
            }

            bag[v] = current;

            if (language[v] == c)
                answer[v] = (current == nullptr ? 0 : current->value);
        }

        for (int v = 0; v < N; ++v)
            if (language[v] == c)
                consider(v, answer[v]);

        delete bag[0];
    }

    cout << bestPeople << ' ' << bestSwitches << '\n';
    return 0;
}