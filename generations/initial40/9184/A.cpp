#include <bits/stdc++.h>
using namespace std;

struct Bag {
    unordered_map<int, int> cnt;
    int value = 0;

    Bag() {
        cnt.max_load_factor(0.7f);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    cin >> n >> k;

    vector<int> lang(n);
    for (int &x : lang) cin >> x;

    vector<vector<int>> children(n);
    for (int i = 1; i < n; ++i) {
        int p;
        cin >> p;
        children[p].push_back(i);
    }

    vector<int> tin(n), tout(n), dep(n), preorder;
    preorder.reserve(n);
    vector<vector<int>> depthEuler;
    vector<vector<int>> colorEuler(k);

    int timer = 0;
    vector<pair<int, int>> st;
    st.push_back({0, 0});

    while (!st.empty()) {
        int v = st.back().first;
        int &state = st.back().second;

        if (state == 0) {
            tin[v] = timer++;
            preorder.push_back(v);

            if ((int)depthEuler.size() <= dep[v])
                depthEuler.resize(dep[v] + 1);

            depthEuler[dep[v]].push_back(tin[v]);
            colorEuler[lang[v]].push_back(tin[v]);
        }

        if (state < (int)children[v].size()) {
            int u = children[v][state++];
            dep[u] = dep[v] + 1;
            st.push_back({u, 0});
        } else {
            tout[v] = timer;
            st.pop_back();
        }
    }

    vector<vector<pair<int, int>>> colorDepth(k);
    for (int v = 0; v < n; ++v)
        colorDepth[lang[v]].push_back({dep[v], 1});

    for (int c = 0; c < k; ++c) {
        sort(colorDepth[c].begin(), colorDepth[c].end());
        vector<pair<int, int>> compressed;
        for (auto x : colorDepth[c]) {
            if (compressed.empty() || compressed.back().first != x.first)
                compressed.push_back(x);
            else
                compressed.back().second++;
        }
        colorDepth[c].swap(compressed);
    }

    auto countInSubtree = [&](const vector<int> &positions, int v) {
        return int(lower_bound(positions.begin(), positions.end(), tout[v]) -
                   lower_bound(positions.begin(), positions.end(), tin[v]));
    };

    const int BLOCK = 700;
    vector<char> heavy(k, false);
    for (int c = 0; c < k; ++c)
        heavy[c] = ((int)colorEuler[c].size() > BLOCK);

    int bestPeople = 0;
    int bestSwitches = INT_MAX;

    auto consider = [&](int v, int people) {
        int alreadyMatching = countInSubtree(colorEuler[lang[v]], v);
        int switches = people - alreadyMatching;

        if (people > bestPeople ||
            (people == bestPeople && switches < bestSwitches)) {
            bestPeople = people;
            bestSwitches = switches;
        }
    };

    // Evaluate rare languages directly.
    for (int c = 0; c < k; ++c) {
        if (heavy[c]) continue;

        for (int v = 0; v < n; ++v) {
            if (lang[v] != c) continue;

            int people = 0;
            for (auto item : colorDepth[c]) {
                int d = item.first;
                int available = item.second;
                int positions = countInSubtree(depthEuler[d], v);
                people += min(positions, available);
            }
            consider(v, people);
        }
    }

    int maxDepth = (int)depthEuler.size() - 1;

    // For every frequent language, build subtree depth counts with
    // small-to-large merging and save answers for leads of that language.
    for (int c = 0; c < k; ++c) {
        if (!heavy[c]) continue;

        vector<int> capacity(maxDepth + 1, 0);
        for (auto item : colorDepth[c])
            capacity[item.first] = item.second;

        vector<Bag *> bags(n, nullptr);
        vector<int> answer(n, 0);

        for (int idx = n - 1; idx >= 0; --idx) {
            int v = preorder[idx];
            Bag *cur = nullptr;

            for (int u : children[v]) {
                Bag *other = bags[u];
                if (other == nullptr) continue;

                if (cur == nullptr) {
                    cur = other;
                    continue;
                }

                if (cur->cnt.size() < other->cnt.size())
                    swap(cur, other);

                for (const auto &entry : other->cnt) {
                    int d = entry.first;
                    int add = entry.second;
                    int old = 0;

                    auto it = cur->cnt.find(d);
                    if (it != cur->cnt.end()) old = it->second;

                    int cap = capacity[d];
                    cur->value += min(old + add, cap) - min(old, cap);
                    cur->cnt[d] = old + add;
                }
                delete other;
            }

            if (capacity[dep[v]] > 0) {
                if (cur == nullptr) cur = new Bag();

                int old = 0;
                auto it = cur->cnt.find(dep[v]);
                if (it != cur->cnt.end()) old = it->second;

                cur->value += min(old + 1, capacity[dep[v]]) -
                              min(old, capacity[dep[v]]);
                cur->cnt[dep[v]] = old + 1;
            }

            bags[v] = cur;
            if (lang[v] == c)
                answer[v] = (cur == nullptr ? 0 : cur->value);
        }

        for (int v = 0; v < n; ++v)
            if (lang[v] == c)
                consider(v, answer[v]);

        delete bags[0];
    }

    cout << bestPeople << ' ' << bestSwitches << '\n';
    return 0;
}
