#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

const int64 INF = (1LL << 60);
const int64 NEG_INF = -(1LL << 60);

struct Node {
    int previous;
    char direction;
};

struct State {
    int64 rightmostRight;
    int64 deadline;
    int node;
};

static bool dominates(const State& a, const State& b) {
    return a.rightmostRight >= b.rightmostRight &&
           a.deadline >= b.deadline;
}

static void prune(vector<State>& states) {
    vector<State> kept;

    for (int i = 0; i < (int)states.size(); ++i) {
        bool isDominated = false;

        for (int j = 0; j < (int)states.size(); ++j) {
            if (i == j) continue;

            if (dominates(states[j], states[i]) &&
                (states[j].rightmostRight > states[i].rightmostRight ||
                 states[j].deadline > states[i].deadline ||
                 j < i)) {
                isDominated = true;
                break;
            }
        }

        if (!isDominated) kept.push_back(states[i]);
    }

    states.swap(kept);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M;
    cin >> N >> M;

    vector<int64> sprinklers(N), flowers(M);
    for (int64& x : sprinklers) cin >> x;
    for (int64& x : flowers) cin >> x;

    auto feasible = [&](int64 K, bool reconstruct) -> pair<bool, string> {
        vector<Node> nodes;
        if (reconstruct) nodes.reserve(2 * N + 5);

        vector<State> states = {{NEG_INF, INF, -1}};
        int flowerIndex = 0;

        auto processFlowers = [&](int begin, int end) {
            for (State& state : states) {
                int64 rightReach = state.rightmostRight == NEG_INF
                    ? NEG_INF
                    : state.rightmostRight + K;

                for (int j = begin; j < end; ++j) {
                    if (flowers[j] > rightReach) {
                        state.deadline =
                            min(state.deadline, flowers[j] + K);
                        break;
                    }
                }
            }
            prune(states);
        };

        for (int i = 0; i < N; ++i) {
            int64 position = sprinklers[i];

            int begin = flowerIndex;
            while (flowerIndex < M && flowers[flowerIndex] < position) {
                ++flowerIndex;
            }
            processFlowers(begin, flowerIndex);

            vector<State> eligible;
            for (const State& state : states) {
                if (state.deadline >= position) {
                    eligible.push_back(state);
                }
            }

            if (eligible.empty()) return {false, ""};

            int bestLeft = 0;
            int bestRight = 0;

            for (int j = 1; j < (int)eligible.size(); ++j) {
                if (eligible[j].rightmostRight >
                    eligible[bestLeft].rightmostRight) {
                    bestLeft = j;
                }
                if (eligible[j].deadline > eligible[bestRight].deadline) {
                    bestRight = j;
                }
            }

            vector<State> nextStates;

            int leftNode = -1;
            if (reconstruct) {
                leftNode = (int)nodes.size();
                nodes.push_back({eligible[bestLeft].node, 'L'});
            }
            nextStates.push_back({
                eligible[bestLeft].rightmostRight,
                INF,
                leftNode
            });

            int rightNode = -1;
            if (reconstruct) {
                rightNode = (int)nodes.size();
                nodes.push_back({eligible[bestRight].node, 'R'});
            }
            nextStates.push_back({
                position,
                eligible[bestRight].deadline,
                rightNode
            });

            prune(nextStates);
            states.swap(nextStates);

            while (flowerIndex < M && flowers[flowerIndex] == position) {
                ++flowerIndex;
            }
        }

        processFlowers(flowerIndex, M);

        int finalState = -1;
        for (int i = 0; i < (int)states.size(); ++i) {
            if (states[i].deadline == INF) {
                finalState = i;
                break;
            }
        }

        if (finalState == -1) return {false, ""};
        if (!reconstruct) return {true, ""};

        string directions(N, 'L');
        int node = states[finalState].node;

        for (int i = N - 1; i >= 0; --i) {
            directions[i] = nodes[node].direction;
            node = nodes[node].previous;
        }

        return {true, directions};
    };

    const int64 MAX_K = 1000000000LL;

    if (!feasible(MAX_K, false).first) {
        cout << -1 << '\n';
        return 0;
    }

    int64 low = 0;
    int64 high = MAX_K;

    while (low < high) {
        int64 mid = low + (high - low) / 2;
        if (feasible(mid, false).first) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }

    auto answer = feasible(low, true);
    cout << low << '\n';
    cout << answer.second << '\n';

    return 0;
}
