#include <algorithm>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <vector>

using namespace std;

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

        /*
        The ordinary transition graph has one vertex for every walk.
        There is an arc i -> j exactly when:
          end(i) = start(j) and color(i) != color(j).

        An interesting tour is precisely a directed cycle in this graph.
        The transition graph can have quadratically many arcs, so we represent
        all outgoing walks of each meeting point by color groups.

        For the color groups g_0, ..., g_{d-1} of a point:
          P_i reaches all walks in groups g_0 ... g_i,
          S_i reaches all walks in groups g_i ... g_{d-1}.

        Therefore a walk whose color is in group g_i can reach all differently
        colored outgoing walks through P_{i-1} and S_{i+1}. If its color is not
        present, it reaches every outgoing walk through P_{d-1}.
        */

        vector<int> order(m);
        iota(order.begin(), order.end(), 0);
        sort(order.begin(), order.end(), [&](int a, int b) {
            if (from[a] != from[b]) return from[a] < from[b];
            if (color[a] != color[b]) return color[a] < color[b];
            return a < b;
        });

        vector<int> groupsPerVertex(n, 0);
        vector<int> groupColor;
        vector<int> groupStart;
        vector<uint8_t> firstGroupOfVertex;

        groupColor.reserve(m);
        groupStart.reserve(m + 1);
        firstGroupOfVertex.reserve(m);

        int pos = 0;
        int previousVertex = -1;

        while (pos < m) {
            int edge = order[pos];
            int vertex = from[edge];
            int col = color[edge];
            int nextPos = pos + 1;

            while (nextPos < m &&
                   from[order[nextPos]] == vertex &&
                   color[order[nextPos]] == col) {
                ++nextPos;
            }

            groupStart.push_back(pos);
            groupColor.push_back(col);
            firstGroupOfVertex.push_back(vertex != previousVertex);
            ++groupsPerVertex[vertex];

            previousVertex = vertex;
            pos = nextPos;
        }

        groupStart.push_back(m);
        int groupCount = static_cast<int>(groupColor.size());

        vector<int> vertexGroupStart(n + 1, 0);
        for (int v = 0; v < n; ++v) {
            vertexGroupStart[v + 1] =
                vertexGroupStart[v] + groupsPerVertex[v];
        }

        // 'from' is no longer needed after the outgoing groups are built.
        vector<int>().swap(from);
        vector<int>().swap(groupsPerVertex);

        // Node IDs:
        // [0, m)                       original walks
        // [m, m + groupCount)          prefix nodes P
        // [m + groupCount, m + 2D)     suffix nodes S
        int nodeCount = m + 2 * groupCount;

        vector<uint8_t> state(nodeCount, 0); // 0=unseen, 1=active, 2=done
        vector<int> parent(nodeCount, -1);
        vector<int> iteratorPosition(nodeCount, 0);

        auto nextNeighbor = [&](int node) -> int {
            int index = iteratorPosition[node]++;

            if (node < m) {
                int vertex = to[node];
                int left = vertexGroupStart[vertex];
                int right = vertexGroupStart[vertex + 1];

                if (left == right) return -1;

                int found = static_cast<int>(
                    lower_bound(groupColor.begin() + left,
                                groupColor.begin() + right,
                                color[node]) -
                    groupColor.begin()
                );

                int firstTarget = -1;
                int secondTarget = -1;

                if (found < right && groupColor[found] == color[node]) {
                    if (found > left) {
                        firstTarget = m + (found - 1);
                    }
                    if (found + 1 < right) {
                        int target = m + groupCount + (found + 1);
                        if (firstTarget == -1) firstTarget = target;
                        else secondTarget = target;
                    }
                } else {
                    // This color is absent, so every outgoing color is allowed.
                    firstTarget = m + (right - 1);
                }

                if (index == 0) return firstTarget;
                if (index == 1) return secondTarget;
                return -1;
            }

            if (node < m + groupCount) {
                int group = node - m;
                bool hasPrevious =
                    (firstGroupOfVertex[group] == 0);

                if (hasPrevious) {
                    if (index == 0) return m + group - 1;
                    --index;
                }

                int size = groupStart[group + 1] - groupStart[group];
                if (index < size) {
                    return order[groupStart[group] + index];
                }
                return -1;
            }

            int group = node - (m + groupCount);
            bool hasNext =
                group + 1 < groupCount &&
                firstGroupOfVertex[group + 1] == 0;

            if (hasNext) {
                if (index == 0) {
                    return m + groupCount + group + 1;
                }
                --index;
            }

            int size = groupStart[group + 1] - groupStart[group];
            if (index < size) {
                return order[groupStart[group] + index];
            }
            return -1;
        };

        vector<int> answer;
        bool foundCycle = false;

        for (int root = 0; root < nodeCount && !foundCycle; ++root) {
            if (state[root] != 0) continue;

            int current = root;
            state[current] = 1;
            parent[current] = -1;

            while (true) {
                int next = nextNeighbor(current);

                if (next == -1) {
                    state[current] = 2;
                    if (current == root) break;
                    current = parent[current];
                    continue;
                }

                if (state[next] == 0) {
                    parent[next] = current;
                    state[next] = 1;
                    current = next;
                    continue;
                }

                if (state[next] == 1) {
                    // The tree path next ... current, followed by current -> next,
                    // is a directed cycle in the compact graph.
                    vector<int> compactCycle;
                    int node = current;
                    compactCycle.push_back(node);

                    while (node != next) {
                        node = parent[node];
                        compactCycle.push_back(node);
                    }

                    reverse(compactCycle.begin(), compactCycle.end());

                    for (int cycleNode : compactCycle) {
                        if (cycleNode < m) {
                            answer.push_back(cycleNode);
                        }
                    }

                    if (answer.size() >= 2) {
                        foundCycle = true;
                        break;
                    }

                    // With x_i != y_i, a compact cycle cannot contain only one
                    // original walk. This is only a defensive fallback.
                    answer.clear();
                }
            }
        }

        if (!foundCycle) {
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
