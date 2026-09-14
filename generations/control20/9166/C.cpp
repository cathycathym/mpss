#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

static const int64 INF = 4000000000000000000LL;
static const int64 NEG_INF = -4000000000000000000LL;

struct Node {
    int previous;
    char direction;
};

struct State {
    int64 rightmost;
    int64 deadline;
    int node;
};

class Solver {
private:
    int n, m;
    vector<int64> sprinklers;
    vector<int64> flowers;

    static bool dominates(const State& a, const State& b) {
        return a.rightmost >= b.rightmost &&
               a.deadline >= b.deadline;
    }

    static void prune(vector<State>& states) {
        vector<State> kept;

        for (int i = 0; i < (int)states.size(); ++i) {
            bool dominated = false;

            for (int j = 0; j < (int)states.size(); ++j) {
                if (i == j) continue;

                if (dominates(states[j], states[i])) {
                    if (states[j].rightmost > states[i].rightmost ||
                        states[j].deadline > states[i].deadline ||
                        j < i) {
                        dominated = true;
                        break;
                    }
                }
            }

            if (!dominated) kept.push_back(states[i]);
        }

        states.swap(kept);
    }

    static void processFlowerRange(
        vector<State>& states,
        const vector<int64>& flowers,
        int begin,
        int end,
        int64 power
    ) {
        for (State& state : states) {
            int64 reach;
            if (state.rightmost == NEG_INF) {
                reach = NEG_INF;
            } else {
                reach = state.rightmost + power;
            }

            for (int i = begin; i < end; ++i) {
                if (flowers[i] > reach) {
                    state.deadline =
                        min(state.deadline, flowers[i] + power);
                    break;
                }
            }
        }

        prune(states);
    }

    pair<bool, string> check(int64 power, bool reconstruct) const {
        vector<Node> nodes;
        if (reconstruct) nodes.reserve(2 * n + 5);

        vector<State> states;
        states.push_back({NEG_INF, INF, -1});

        int flowerIndex = 0;

        for (int i = 0; i < n; ++i) {
            int64 position = sprinklers[i];

            int before = flowerIndex;
            while (flowerIndex < m && flowers[flowerIndex] < position) {
                ++flowerIndex;
            }

            processFlowerRange(
                states, flowers, before, flowerIndex, power
            );

            vector<State> eligible;
            for (const State& state : states) {
                if (state.deadline >= position) {
                    eligible.push_back(state);
                }
            }

            if (eligible.empty()) return {false, ""};

            int bestForLeft = 0;
            int bestForRight = 0;

            for (int j = 1; j < (int)eligible.size(); ++j) {
                if (eligible[j].rightmost >
                    eligible[bestForLeft].rightmost) {
                    bestForLeft = j;
                }

                if (eligible[j].deadline >
                    eligible[bestForRight].deadline) {
                    bestForRight = j;
                }
            }

            vector<State> nextStates;

            int leftNode = -1;
            if (reconstruct) {
                leftNode = (int)nodes.size();
                nodes.push_back({
                    eligible[bestForLeft].node, 'L'
                });
            }

            nextStates.push_back({
                eligible[bestForLeft].rightmost,
                INF,
                leftNode
            });

            int rightNode = -1;
            if (reconstruct) {
                rightNode = (int)nodes.size();
                nodes.push_back({
                    eligible[bestForRight].node, 'R'
                });
            }

            nextStates.push_back({
                position,
                eligible[bestForRight].deadline,
                rightNode
            });

            prune(nextStates);
            states.swap(nextStates);

            // A flower exactly at a sprinkler position is covered
            // regardless of that sprinkler's orientation.
            while (flowerIndex < m &&
                   flowers[flowerIndex] == position) {
                ++flowerIndex;
            }
        }

        processFlowerRange(
            states, flowers, flowerIndex, m, power
        );

        int finalState = -1;
        for (int i = 0; i < (int)states.size(); ++i) {
            if (states[i].deadline == INF) {
                finalState = i;
                break;
            }
        }

        if (finalState == -1) return {false, ""};
        if (!reconstruct) return {true, ""};

        string answer(n, 'L');
        int node = states[finalState].node;

        for (int i = n - 1; i >= 0; --i) {
            answer[i] = nodes[node].direction;
            node = nodes[node].previous;
        }

        return {true, answer};
    }

public:
    void solve() {
        ios::sync_with_stdio(false);
        cin.tie(nullptr);

        cin >> n >> m;

        sprinklers.resize(n);
        flowers.resize(m);

        for (int i = 0; i < n; ++i) cin >> sprinklers[i];
        for (int i = 0; i < m; ++i) cin >> flowers[i];

        if (!check(1000000000LL, false).first) {
            cout << -1 << '\n';
            return;
        }

        int64 low = 0;
        int64 high = 1000000000LL;

        while (low < high) {
            int64 middle = low + (high - low) / 2;

            if (check(middle, false).first) {
                high = middle;
            } else {
                low = middle + 1;
            }
        }

        auto result = check(low, true);

        cout << low << '\n';
        cout << result.second << '\n';
    }
};

int main() {
    Solver solver;
    solver.solve();
    return 0;
}
