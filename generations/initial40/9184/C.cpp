#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <cmath>

using namespace std;

struct Bag {
    unordered_map<int, int> cnt;
    int value;
    Bag() : value(0) {}
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, K;
    cin >> N >> K;

    vector<int> language(N);
    for (int i = 0; i < N; ++i) cin >> language[i];

    vector<vector<int> > children(N);
    for (int i = 1; i < N; ++i) {
        int boss;
        cin >> boss;
        children[boss].push_back(i);
    }

    vector<int> depth(N), tin(N), tout(N), postorder;
    vector<vector<int> > atDepth;
    vector<vector<int> > langTin(K), langNodes(K);

    int timer = 0, maxDepth = 0;
    vector<pair<int, int> > st;
    st.push_back(make_pair(0, 0));

    while (!st.empty()) {
        int v = st.back().first;
        int state = st.back().second;
        st.pop_back();

        if (state == 0) {
            tin[v] = timer++;
            if ((int)atDepth.size() <= depth[v])
                atDepth.resize(depth[v] + 1);
            atDepth[depth[v]].push_back(tin[v]);

            langTin[language[v]].push_back(tin[v]);
            langNodes[language[v]].push_back(v);
            maxDepth = max(maxDepth, depth[v]);

            st.push_back(make_pair(v, 1));
            for (int i = (int)children[v].size() - 1; i >= 0; --i) {
                int u = children[v][i];
                depth[u] = depth[v] + 1;
                st.push_back(make_pair(u, 0));
            }
        } else {
            tout[v] = timer - 1;
            postorder.push_back(v);
        }
    }

    vector<vector<pair<int, int> > > languageDepths(K);
    vector<int> frequency(K);

    for (int c = 0; c < K; ++c) {
        frequency[c] = (int)langNodes[c].size();
        sort(langNodes[c].begin(), langNodes[c].end(),
             [&](int a, int b) {
                 if (depth[a] != depth[b]) return depth[a] < depth[b];
                 return a < b;
             });

        for (int v : langNodes[c]) {
            if (languageDepths[c].empty() ||
                languageDepths[c].back().first != depth[v]) {
                languageDepths[c].push_back(make_pair(depth[v], 1));
            } else {
                ++languageDepths[c].back().second;
            }
        }
    }

    auto countInSubtree = [&](int c, int left, int right) {
        const vector<int>& a = langTin[c];
        return (int)(upper_bound(a.begin(), a.end(), right) -
                     lower_bound(a.begin(), a.end(), left));
    };

    auto countAtDepthInSubtree = [&](int d, int left, int right) {
        const vector<int>& a = atDepth[d];
        return (int)(upper_bound(a.begin(), a.end(), right) -
                     lower_bound(a.begin(), a.end(), left));
    };

    int threshold = (int)sqrt((double)N) + 1;
    int bestPeople = 0;
    int bestSwitches = N + 1;

    auto consider = [&](int v, int people) {
        int original = countInSubtree(language[v], tin[v], tout[v]);
        int switches = people - original;
        if (people > bestPeople ||
            (people == bestPeople && switches < bestSwitches)) {
            bestPeople = people;
            bestSwitches = switches;
        }
    };

    // Rare languages: enumerate their occupied depths directly.
    for (int v = 0; v < N; ++v) {
        int c = language[v];
        if (frequency[c] > threshold) continue;

        int people = 0;
        for (const pair<int, int>& item : languageDepths[c]) {
            int d = item.first;
            if (d < depth[v]) continue;

            int capacity = countAtDepthInSubtree(d, tin[v], tout[v]);
            people += min(capacity, item.second);
        }
        consider(v, people);
    }

    // Frequent languages: run the subtree-map DP once per language.
    for (int c = 0; c < K; ++c) {
        if (frequency[c] <= threshold) continue;

        vector<int> available(maxDepth + 1, 0);
        for (const pair<int, int>& item : languageDepths[c])
            available[item.first] = item.second;

        vector<Bag*> bag(N, nullptr);

        for (int v : postorder) {
            Bag* current = nullptr;
            int largestChild = -1;

            for (int u : children[v]) {
                if (largestChild == -1 ||
                    bag[u]->cnt.size() > bag[largestChild]->cnt.size()) {
                    largestChild = u;
                }
            }

            if (largestChild == -1) {
                current = new Bag();
            } else {
                current = bag[largestChild];
            }

            for (int u : children[v]) {
                if (u == largestChild) continue;

                for (const auto& entry : bag[u]->cnt) {
                    int d = entry.first;
                    int add = entry.second;

                    auto it = current->cnt.find(d);
                    int oldCount = (it == current->cnt.end() ? 0 : it->second);
                    int newCount = oldCount + add;

                    current->value += min(newCount, available[d])
                                    - min(oldCount, available[d]);

                    if (it == current->cnt.end())
                        current->cnt.emplace(d, newCount);
                    else
                        it->second = newCount;
                }
                delete bag[u];
            }

            int d = depth[v];
            auto it = current->cnt.find(d);
            int oldCount = (it == current->cnt.end() ? 0 : it->second);
            int newCount = oldCount + 1;

            current->value += min(newCount, available[d])
                            - min(oldCount, available[d]);

            if (it == current->cnt.end())
                current->cnt.emplace(d, newCount);
            else
                it->second = newCount;

            bag[v] = current;

            if (language[v] == c)
                consider(v, current->value);
        }

        delete bag[0];
    }

    cout << bestPeople << ' ' << bestSwitches << '\n';
    return 0;
}
