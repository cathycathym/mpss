#include <bits/stdc++.h>
using namespace std;

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

        // Create one group for every distinct (starting vertex, color).
        vector<int> order(m);
        iota(order.begin(), order.end(), 0);

        sort(order.begin(), order.end(), [&](int a, int b) {
            if (from[a] != from[b]) return from[a] < from[b];
            if (color[a] != color[b]) return color[a] < color[b];
            return a < b;
        });

        vector<int> groupOf(m);
        vector<int> groupVertex;
        vector<int> groupColor;
        vector<int> whiteGroupHead;

        vector<int> whiteNext(m, -1), whitePrev(m, -1);

        int lastVertex = -1, lastColor = -1;
        int groups = 0;

        for (int e : order) {
            if (groups == 0 || from[e] != lastVertex || color[e] != lastColor) {
                lastVertex = from[e];
                lastColor = color[e];

                groupVertex.push_back(from[e]);
                groupColor.push_back(color[e]);
                whiteGroupHead.push_back(-1);
                ++groups;
            }

            int g = groups - 1;
            groupOf[e] = g;

            whiteNext[e] = whiteGroupHead[g];
            if (whiteGroupHead[g] != -1)
                whitePrev[whiteGroupHead[g]] = e;
            whiteGroupHead[g] = e;
        }

        // Linked lists of nonempty white groups for every starting vertex.
        vector<int> whiteVertexHead(n, -1);
        vector<int> whiteGroupNext(groups, -1), whiteGroupPrev(groups, -1);

        for (int g = 0; g < groups; ++g) {
            int v = groupVertex[g];
            whiteGroupNext[g] = whiteVertexHead[v];
            if (whiteVertexHead[v] != -1)
                whiteGroupPrev[whiteVertexHead[v]] = g;
            whiteVertexHead[v] = g;
        }

        // Active (gray) walks, also grouped by (starting vertex, color).
        vector<int> grayVertexHead(n, -1);
        vector<int> grayGroupHead(groups, -1);
        vector<int> grayGroupNext(groups, -1), grayGroupPrev(groups, -1);
        vector<int> grayNext(m, -1);

        vector<unsigned char> state(m, 0); // 0 white, 1 gray, 2 black
        vector<int> parent(m, -1);

        auto detachWhiteGroup = [&](int g) {
            int v = groupVertex[g];
            int p = whiteGroupPrev[g];
            int q = whiteGroupNext[g];

            if (p != -1) whiteGroupNext[p] = q;
            else whiteVertexHead[v] = q;

            if (q != -1) whiteGroupPrev[q] = p;

            whiteGroupPrev[g] = whiteGroupNext[g] = -1;
        };

        auto removeWhiteEdge = [&](int e) {
            int g = groupOf[e];
            int p = whitePrev[e];
            int q = whiteNext[e];

            if (p != -1) whiteNext[p] = q;
            else whiteGroupHead[g] = q;

            if (q != -1) whitePrev[q] = p;

            whitePrev[e] = whiteNext[e] = -1;

            if (whiteGroupHead[g] == -1)
                detachWhiteGroup(g);
        };

        auto chooseWhiteSuccessor = [&](int vertex, int forbiddenColor) {
            int g = whiteVertexHead[vertex];

            // There is at most one group with the forbidden color.
            if (g != -1 && groupColor[g] == forbiddenColor)
                g = whiteGroupNext[g];

            if (g == -1) return -1;
            return whiteGroupHead[g];
        };

        auto addGrayEdge = [&](int e) {
            int g = groupOf[e];

            if (grayGroupHead[g] == -1) {
                int v = groupVertex[g];
                grayGroupPrev[g] = -1;
                grayGroupNext[g] = grayVertexHead[v];

                if (grayVertexHead[v] != -1)
                    grayGroupPrev[grayVertexHead[v]] = g;

                grayVertexHead[v] = g;
            }

            grayNext[e] = grayGroupHead[g];
            grayGroupHead[g] = e;
        };

        auto removeGrayEdge = [&](int e) {
            int g = groupOf[e];

            // Gray edges of one group are removed in DFS-stack order.
            grayGroupHead[g] = grayNext[e];
            grayNext[e] = -1;

            if (grayGroupHead[g] == -1) {
                int v = groupVertex[g];
                int p = grayGroupPrev[g];
                int q = grayGroupNext[g];

                if (p != -1) grayGroupNext[p] = q;
                else grayVertexHead[v] = q;

                if (q != -1) grayGroupPrev[q] = p;

                grayGroupPrev[g] = grayGroupNext[g] = -1;
            }
        };

        auto chooseGraySuccessor = [&](int vertex, int forbiddenColor) {
            int g = grayVertexHead[vertex];

            if (g != -1 && groupColor[g] == forbiddenColor)
                g = grayGroupNext[g];

            if (g == -1) return -1;
            return grayGroupHead[g];
        };

        vector<int> answer;
        vector<int> dfsStack;

        for (int root = 0; root < m && answer.empty(); ++root) {
            if (state[root] != 0)
                continue;

            removeWhiteEdge(root);
            state[root] = 1;
            parent[root] = -1;
            addGrayEdge(root);
            dfsStack.push_back(root);

            while (!dfsStack.empty() && answer.empty()) {
                int e = dfsStack.back();

                // Any gray successor is an ancestor and closes a cycle.
                int ancestor = chooseGraySuccessor(to[e], color[e]);
                if (ancestor != -1) {
                    int cur = e;
                    while (cur != ancestor) {
                        answer.push_back(cur);
                        cur = parent[cur];
                    }
                    answer.push_back(ancestor);
                    reverse(answer.begin(), answer.end());
                    break;
                }

                int nextEdge = chooseWhiteSuccessor(to[e], color[e]);

                if (nextEdge != -1) {
                    removeWhiteEdge(nextEdge);
                    state[nextEdge] = 1;
                    parent[nextEdge] = e;
                    addGrayEdge(nextEdge);
                    dfsStack.push_back(nextEdge);
                } else {
                    removeGrayEdge(e);
                    state[e] = 2;
                    dfsStack.pop_back();
                }
            }

            // Only needed when a cycle was found before normal stack cleanup.
            if (!answer.empty())
                break;
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
