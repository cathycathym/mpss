#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
using namespace std;

struct Group {
    int vertex, color;
    int left, pointer, remaining;

    int nextWhite, prevWhite;
    int nextActive, prevActive;
    int activeTop;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int t;
    cin >> t;

    while (t--) {
        int n, m;
        cin >> n >> m;

        vector<int> from(m), to(m), color(m);
        for (int i = 0; i < m; ++i) {
            cin >> from[i] >> to[i] >> color[i];
            --from[i];
            --to[i];
        }

        vector<int> order(m), temp(m);
        iota(order.begin(), order.end(), 0);

        vector<int> count(max(n, m) + 1, 0);

        for (int e : order)
            ++count[color[e]];

        int prefix = 0;
        for (int c = 0; c <= m; ++c) {
            int amount = count[c];
            count[c] = prefix;
            prefix += amount;
        }

        for (int e : order)
            temp[count[color[e]]++] = e;

        fill(count.begin(), count.end(), 0);
        for (int e : temp)
            ++count[from[e]];

        prefix = 0;
        for (int v = 0; v < n; ++v) {
            int amount = count[v];
            count[v] = prefix;
            prefix += amount;
        }

        for (int e : temp)
            order[count[from[e]]++] = e;

        vector<Group> groups;
        groups.reserve(m);

        vector<int> groupOf(m);
        vector<int> whiteHead(n, -1);
        vector<int> activeHead(n, -1);

        for (int left = 0; left < m;) {
            int right = left + 1;
            int first = order[left];

            while (right < m &&
                   from[order[right]] == from[first] &&
                   color[order[right]] == color[first]) {
                ++right;
            }

            int id = static_cast<int>(groups.size());
            Group group;

            group.vertex = from[first];
            group.color = color[first];
            group.left = left;
            group.pointer = right - 1;
            group.remaining = right - left;

            group.prevWhite = -1;
            group.nextWhite = whiteHead[group.vertex];
            group.prevActive = -1;
            group.nextActive = -1;
            group.activeTop = -1;

            groups.push_back(group);

            if (whiteHead[group.vertex] != -1)
                groups[whiteHead[group.vertex]].prevWhite = id;
            whiteHead[group.vertex] = id;

            for (int i = left; i < right; ++i)
                groupOf[order[i]] = id;

            left = right;
        }

        vector<unsigned char> state(m, 0);
        vector<int> parent(m, -1);
        vector<int> previousActive(m, -1);
        vector<int> stack;
        vector<int> answer;

        stack.reserve(m);
        answer.reserve(m);

        auto unlinkWhiteGroup = [&](int id) {
            Group &g = groups[id];

            if (g.prevWhite == -1)
                whiteHead[g.vertex] = g.nextWhite;
            else
                groups[g.prevWhite].nextWhite = g.nextWhite;

            if (g.nextWhite != -1)
                groups[g.nextWhite].prevWhite = g.prevWhite;

            g.prevWhite = g.nextWhite = -1;
        };

        auto removeWhiteEdge = [&](int e) {
            Group &g = groups[groupOf[e]];
            if (--g.remaining == 0)
                unlinkWhiteGroup(groupOf[e]);
        };

        auto activate = [&](int e) {
            int id = groupOf[e];
            Group &g = groups[id];

            previousActive[e] = g.activeTop;

            if (g.activeTop == -1) {
                g.prevActive = -1;
                g.nextActive = activeHead[g.vertex];

                if (g.nextActive != -1)
                    groups[g.nextActive].prevActive = id;

                activeHead[g.vertex] = id;
            }

            g.activeTop = e;
        };

        auto deactivate = [&](int e) {
            int id = groupOf[e];
            Group &g = groups[id];

            g.activeTop = previousActive[e];

            if (g.activeTop == -1) {
                if (g.prevActive == -1)
                    activeHead[g.vertex] = g.nextActive;
                else
                    groups[g.prevActive].nextActive = g.nextActive;

                if (g.nextActive != -1)
                    groups[g.nextActive].prevActive = g.prevActive;

                g.prevActive = g.nextActive = -1;
            }
        };

        auto discover = [&](int e, int p) {
            state[e] = 1;
            parent[e] = p;
            activate(e);
            stack.push_back(e);
        };

        for (int root = 0; root < m && answer.empty(); ++root) {
            if (state[root] != 0)
                continue;

            removeWhiteEdge(root);
            discover(root, -1);

            while (!stack.empty() && answer.empty()) {
                int current = stack.back();
                int vertex = to[current];
                int forbidden = color[current];

                int groupId = whiteHead[vertex];
                if (groupId != -1 && groups[groupId].color == forbidden)
                    groupId = groups[groupId].nextWhite;

                if (groupId != -1) {
                    Group &g = groups[groupId];

                    while (g.pointer >= g.left &&
                           state[order[g.pointer]] != 0) {
                        --g.pointer;
                    }

                    int nextEdge = order[g.pointer--];
                    removeWhiteEdge(nextEdge);
                    discover(nextEdge, current);
                    continue;
                }

                int activeGroup = activeHead[vertex];
                if (activeGroup != -1 &&
                    groups[activeGroup].color == forbidden) {
                    activeGroup = groups[activeGroup].nextActive;
                }

                if (activeGroup != -1) {
                    int ancestor = groups[activeGroup].activeTop;

                    for (int e = current;; e = parent[e]) {
                        answer.push_back(e);
                        if (e == ancestor)
                            break;
                    }

                    reverse(answer.begin(), answer.end());
                    break;
                }

                deactivate(current);
                state[current] = 2;
                stack.pop_back();
            }
        }

        if (answer.empty()) {
            cout << "NO\n";
        } else {
            cout << "YES\n";
            cout << answer.size();
            for (int e : answer)
                cout << ' ' << e + 1;
            cout << '\n';
        }
    }

    return 0;
}
