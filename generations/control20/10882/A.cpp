#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>

using namespace std;

struct Group {
    int vertex;
    int color;

    int left;
    int pointer;
    int remaining;

    int nextUnvisited;
    int prevUnvisited;

    int nextActive;
    int prevActive;
    int activeTop;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int tests;
    cin >> tests;

    while (tests--) {
        int n, m;
        cin >> n >> m;

        vector<int> from(m), to(m), color(m);
        for (int i = 0; i < m; ++i) {
            cin >> from[i] >> to[i] >> color[i];
            --from[i];
            --to[i];
        }

        // Stable counting sort first by color, then by starting vertex.
        vector<int> order(m), temporary(m);
        iota(order.begin(), order.end(), 0);

        vector<int> count(max(n, m) + 1, 0);

        for (int edge : order) {
            ++count[color[edge]];
        }
        int sum = 0;
        for (int value = 0; value <= m; ++value) {
            int amount = count[value];
            count[value] = sum;
            sum += amount;
        }
        for (int edge : order) {
            temporary[count[color[edge]]++] = edge;
        }

        fill(count.begin(), count.end(), 0);
        for (int edge : temporary) {
            ++count[from[edge]];
        }
        sum = 0;
        for (int vertex = 0; vertex < n; ++vertex) {
            int amount = count[vertex];
            count[vertex] = sum;
            sum += amount;
        }
        for (int edge : temporary) {
            order[count[from[edge]]++] = edge;
        }

        vector<int> groupOf(m);
        vector<int> unvisitedHead(n, -1);
        vector<int> activeHead(n, -1);
        vector<Group> groups;
        groups.reserve(m);

        int position = 0;
        while (position < m) {
            int end = position + 1;
            int firstEdge = order[position];

            while (end < m &&
                   from[order[end]] == from[firstEdge] &&
                   color[order[end]] == color[firstEdge]) {
                ++end;
            }

            int groupId = static_cast<int>(groups.size());
            Group group;
            group.vertex = from[firstEdge];
            group.color = color[firstEdge];
            group.left = position;
            group.pointer = end - 1;
            group.remaining = end - position;

            group.prevUnvisited = -1;
            group.nextUnvisited = unvisitedHead[group.vertex];

            group.prevActive = -1;
            group.nextActive = -1;
            group.activeTop = -1;

            groups.push_back(group);

            if (unvisitedHead[group.vertex] != -1) {
                groups[unvisitedHead[group.vertex]].prevUnvisited = groupId;
            }
            unvisitedHead[group.vertex] = groupId;

            for (int i = position; i < end; ++i) {
                groupOf[order[i]] = groupId;
            }

            position = end;
        }

        // 0 = unvisited, 1 = on the current DFS stack, 2 = finished.
        vector<unsigned char> state(m, 0);
        vector<int> parent(m, -1);
        vector<int> previousActiveInGroup(m, -1);
        vector<int> dfsStack;
        dfsStack.reserve(m);

        auto unlinkUnvisitedGroup = [&](int groupId) {
            Group &group = groups[groupId];

            if (group.prevUnvisited == -1) {
                unvisitedHead[group.vertex] = group.nextUnvisited;
            } else {
                groups[group.prevUnvisited].nextUnvisited =
                    group.nextUnvisited;
            }

            if (group.nextUnvisited != -1) {
                groups[group.nextUnvisited].prevUnvisited =
                    group.prevUnvisited;
            }

            group.nextUnvisited = -1;
            group.prevUnvisited = -1;
        };

        auto removeFromUnvisited = [&](int edge) {
            int groupId = groupOf[edge];
            Group &group = groups[groupId];

            --group.remaining;
            if (group.remaining == 0) {
                unlinkUnvisitedGroup(groupId);
            }
        };

        auto activate = [&](int edge) {
            int groupId = groupOf[edge];
            Group &group = groups[groupId];

            previousActiveInGroup[edge] = group.activeTop;

            if (group.activeTop == -1) {
                group.prevActive = -1;
                group.nextActive = activeHead[group.vertex];

                if (group.nextActive != -1) {
                    groups[group.nextActive].prevActive = groupId;
                }
                activeHead[group.vertex] = groupId;
            }

            group.activeTop = edge;
        };

        auto deactivate = [&](int edge) {
            int groupId = groupOf[edge];
            Group &group = groups[groupId];

            group.activeTop = previousActiveInGroup[edge];

            if (group.activeTop == -1) {
                if (group.prevActive == -1) {
                    activeHead[group.vertex] = group.nextActive;
                } else {
                    groups[group.prevActive].nextActive = group.nextActive;
                }

                if (group.nextActive != -1) {
                    groups[group.nextActive].prevActive = group.prevActive;
                }

                group.nextActive = -1;
                group.prevActive = -1;
            }
        };

        auto discover = [&](int edge, int parentEdge) {
            state[edge] = 1;
            parent[edge] = parentEdge;
            activate(edge);
            dfsStack.push_back(edge);
        };

        vector<int> answer;

        for (int root = 0; root < m && answer.empty(); ++root) {
            if (state[root] != 0) {
                continue;
            }

            removeFromUnvisited(root);
            discover(root, -1);

            while (!dfsStack.empty() && answer.empty()) {
                int current = dfsStack.back();
                int vertex = to[current];
                int forbiddenColor = color[current];

                // Find a nonempty unvisited group with another color.
                int groupId = unvisitedHead[vertex];
                if (groupId != -1 &&
                    groups[groupId].color == forbiddenColor) {
                    groupId = groups[groupId].nextUnvisited;
                }

                if (groupId != -1) {
                    Group &group = groups[groupId];

                    while (group.pointer >= group.left &&
                           state[order[group.pointer]] != 0) {
                        --group.pointer;
                    }

                    int nextEdge = order[group.pointer];
                    --group.pointer;

                    removeFromUnvisited(nextEdge);
                    discover(nextEdge, current);
                    continue;
                }

                // All unvisited successors have been processed.
                // Look for a valid successor already on the DFS stack.
                int activeGroup = activeHead[vertex];
                if (activeGroup != -1 &&
                    groups[activeGroup].color == forbiddenColor) {
                    activeGroup = groups[activeGroup].nextActive;
                }

                if (activeGroup != -1) {
                    int ancestor = groups[activeGroup].activeTop;

                    int edge = current;
                    while (edge != ancestor) {
                        answer.push_back(edge);
                        edge = parent[edge];
                    }
                    answer.push_back(ancestor);
                    reverse(answer.begin(), answer.end());
                    break;
                }

                deactivate(current);
                state[current] = 2;
                dfsStack.pop_back();
            }
        }

        if (answer.empty()) {
            cout << "NO\n";
        } else {
            cout << "YES\n";
            cout << answer.size();
            for (int edge : answer) {
                cout << ' ' << edge + 1;
            }
            cout << '\n';
        }
    }

    return 0;
}
