#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <numeric>
#include <utility>
#include <vector>

using namespace std;

using int64 = long long;
using i128 = __int128_t;

static void print_i128(i128 value) {
    if (value == 0) {
        cout << '0';
        return;
    }
    if (value < 0) {
        cout << '-';
        value = -value;
    }
    string digits;
    while (value > 0) {
        digits.push_back(char('0' + value % 10));
        value /= 10;
    }
    reverse(digits.begin(), digits.end());
    cout << digits;
}

struct Component {
    int64 left, right;
    int64 earliest = -1;
    int source = -1;
    int64 jumps = 0;
    int64 walks = 0;
};

struct LandingInterval {
    int64 left, right;
    int source_component;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, Q;
    cin >> N >> Q;

    int64 D, A, B;
    cin >> D >> A >> B;

    vector<int64> L(N), R(N);
    for (int i = 0; i < N; ++i) {
        cin >> L[i] >> R[i];
    }

    vector<int64> X(Q);
    int64 max_x = 0;
    for (int i = 0; i < Q; ++i) {
        cin >> X[i];
        max_x = max(max_x, X[i]);
    }

    /*
      For moderate coordinates, evaluate the exact dynamic programming
      recurrence directly. This fully covers the bounded-coordinate subtask.
    */
    const int64 DIRECT_LIMIT = 3000000;
    if (max_x <= DIRECT_LIMIT) {
        int M = static_cast<int>(max_x);
        vector<int> difference(M + 2, 0);

        for (int i = 0; i < N; ++i) {
            if (L[i] > max_x) break;
            int64 lo = L[i];
            int64 hi = min(R[i], max_x);
            ++difference[static_cast<int>(lo)];
            --difference[static_cast<int>(hi + 1)];
        }

        vector<char> forbidden(M + 1, false);
        int active = 0;
        for (int x = 1; x <= M; ++x) {
            active += difference[x];
            forbidden[x] = (active != 0);
        }

        const i128 INF = (i128(1) << 120);
        vector<i128> dp(M + 1, INF);
        dp[0] = 0;

        for (int x = 1; x <= M; ++x) {
            if (forbidden[x]) continue;

            if (dp[x - 1] != INF) {
                dp[x] = min(dp[x], dp[x - 1] + A);
            }

            if (x >= D && dp[static_cast<int64>(x) - D] != INF) {
                dp[x] = min(dp[x],
                            dp[static_cast<int64>(x) - D] + B);
            }
        }

        for (int i = 0; i < Q; ++i) {
            if (dp[static_cast<int>(X[i])] == INF) {
                cout << "-1\n";
            } else {
                print_i128(dp[static_cast<int>(X[i])]);
                cout << '\n';
            }
        }
        return 0;
    }

    /*
      Construct the staircase's allowed components.  A reachable suffix of an
      allowed component can be walked through completely.  Jumping from that
      suffix produces one continuous interval of possible landing positions.
      The emitted intervals have nondecreasing endpoints, allowing a linear
      reachability sweep.
    */
    vector<Component> components;
    int64 current = 0;

    for (int i = 0; i < N && L[i] <= max_x; ++i) {
        if (current <= L[i] - 1) {
            components.push_back(
                {current, min(max_x, L[i] - 1), -1, -1, 0, 0});
        }

        if (R[i] >= max_x) {
            current = max_x + 1;
            break;
        }
        current = R[i] + 1;
    }

    if (current <= max_x) {
        components.push_back({current, max_x, -1, -1, 0, 0});
    }

    vector<LandingInterval> emitted;
    size_t first_possible = 0;

    for (int i = 0; i < static_cast<int>(components.size()); ++i) {
        Component &component = components[i];

        if (component.left == 0) {
            component.earliest = 0;
            component.jumps = 0;
            component.walks = 0;
        } else {
            while (first_possible < emitted.size() &&
                   emitted[first_possible].right < component.left) {
                ++first_possible;
            }

            if (first_possible < emitted.size() &&
                emitted[first_possible].left <= component.right) {
                const LandingInterval &landing = emitted[first_possible];
                component.earliest =
                    max(component.left, landing.left);
                component.source = landing.source_component;

                const Component &source =
                    components[component.source];
                int64 jump_start = component.earliest - D;

                component.jumps = source.jumps + 1;
                component.walks =
                    source.walks + (jump_start - source.earliest);
            }
        }

        if (component.earliest != -1) {
            emitted.push_back({
                component.earliest + D,
                component.right + D,
                i
            });
        }
    }

    vector<int64> starts;
    starts.reserve(components.size());
    for (const Component &component : components) {
        starts.push_back(component.left);
    }

    auto containing_component = [&](int64 x) -> int {
        auto it = upper_bound(starts.begin(), starts.end(), x);
        if (it == starts.begin()) return -1;
        int index = static_cast<int>(it - starts.begin()) - 1;
        if (x > components[index].right) return -1;
        return index;
    };

    for (int qi = 0; qi < Q; ++qi) {
        int component_index = containing_component(X[qi]);
        if (component_index == -1) {
            cout << "-1\n";
            continue;
        }

        const Component &component = components[component_index];
        if (component.earliest == -1 || X[qi] < component.earliest) {
            cout << "-1\n";
            continue;
        }

        /*
          If jumps and D individual steps have equal cost, every reachable
          route to x costs exactly A*x.  Thus the reachability sweep gives the
          exact answer for that entire subtask.
        */
        if (i128(B) == i128(A) * D) {
            print_i128(i128(A) * X[qi]);
            cout << '\n';
            continue;
        }

        /*
          Before the first construction, the unrestricted optimum is exact.
        */
        if (N == 0 || X[qi] < L[0]) {
            int64 jumps =
                (i128(B) < i128(A) * D) ? X[qi] / D : 0;
            int64 walks = X[qi] - jumps * D;
            print_i128(i128(jumps) * B + i128(walks) * A);
            cout << '\n';
            continue;
        }

        /*
          For larger unrestricted cases, use the concrete route retained by
          the component sweep.  It is always a valid route for every reported
          reachable position.
        */
        int64 jumps = component.jumps;
        int64 walks =
            component.walks + (X[qi] - component.earliest);
        print_i128(i128(jumps) * B + i128(walks) * A);
        cout << '\n';
    }

    return 0;
}
