#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
using i128 = __int128_t;

static const int64 INF = (int64)4e18;

static void print_i128(i128 x) {
    if (x == 0) {
        cout << "0\n";
        return;
    }
    if (x < 0) {
        cout << '-';
        x = -x;
    }
    string s;
    while (x > 0) {
        s.push_back(char('0' + x % 10));
        x /= 10;
    }
    reverse(s.begin(), s.end());
    cout << s << '\n';
}

struct ReachInterval {
    int64 left, right;
    int64 jumps;
};

struct SegmentTree {
    struct Node {
        int64 first = INF;
        int64 last = INF;
        int64 minimum = INF;
        int64 lazy = INF;
        bool has_lazy = true;
        bool nonincreasing = true;
    };

    int n;
    vector<Node> tree;

    explicit SegmentTree(int n_) : n(n_), tree(4 * max(1, n_)) {}

    void apply(int p, int64 value) {
        tree[p].first = value;
        tree[p].last = value;
        tree[p].minimum = value;
        tree[p].lazy = value;
        tree[p].has_lazy = true;
        tree[p].nonincreasing = true;
    }

    void push(int p) {
        if (!tree[p].has_lazy) return;
        apply(p << 1, tree[p].lazy);
        apply(p << 1 | 1, tree[p].lazy);
        tree[p].has_lazy = false;
    }

    void pull(int p) {
        const Node &a = tree[p << 1];
        const Node &b = tree[p << 1 | 1];

        tree[p].first = a.first;
        tree[p].last = b.last;
        tree[p].minimum = min(a.minimum, b.minimum);
        tree[p].nonincreasing =
            a.nonincreasing && b.nonincreasing && a.last >= b.first;
        tree[p].has_lazy = false;
    }

    void assign_range(int ql, int qr, int64 value) {
        if (ql > qr) return;
        assign_range(1, 0, n - 1, ql, qr, value);
    }

    void assign_range(int p, int l, int r, int ql, int qr, int64 value) {
        if (qr < l || r < ql) return;
        if (ql <= l && r <= qr) {
            apply(p, value);
            return;
        }

        push(p);
        int m = (l + r) >> 1;
        assign_range(p << 1, l, m, ql, qr, value);
        assign_range(p << 1 | 1, m + 1, r, ql, qr, value);
        pull(p);
    }

    int64 get_point(int index) {
        return get_point(1, 0, n - 1, index);
    }

    int64 get_point(int p, int l, int r, int index) {
        if (l == r) return tree[p].first;
        push(p);
        int m = (l + r) >> 1;
        if (index <= m) return get_point(p << 1, l, m, index);
        return get_point(p << 1 | 1, m + 1, r, index);
    }

    void prefix_min(int ql, int qr) {
        if (ql > qr) return;
        int64 carry = INF;
        prefix_min(1, 0, n - 1, ql, qr, carry);
    }

    void prefix_min(int p, int l, int r, int ql, int qr, int64 &carry) {
        if (qr < l || r < ql) return;

        if (ql <= l && r <= qr) {
            Node &node = tree[p];

            if (node.has_lazy) {
                int64 value = min(node.first, carry);
                apply(p, value);
                carry = value;
                return;
            }

            if (node.nonincreasing) {
                if (node.first <= carry) {
                    carry = node.last;
                    return;
                }
                if (node.last >= carry) {
                    apply(p, carry);
                    return;
                }
            }
        }

        if (l == r) {
            int64 value = min(tree[p].first, carry);
            apply(p, value);
            carry = value;
            return;
        }

        push(p);
        int m = (l + r) >> 1;
        prefix_min(p << 1, l, m, ql, qr, carry);
        prefix_min(p << 1 | 1, m + 1, r, ql, qr, carry);
        pull(p);
    }
};

struct RowEvent {
    vector<pair<int64, int64>> blocked;
    vector<int> queries;
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
    int64 max_query = 0;
    for (int i = 0; i < Q; ++i) {
        cin >> X[i];
        max_query = max(max_query, X[i]);
    }

    i128 jump_delta = (i128)B - (i128)A * D;

    /*
     * If jumping is not cheaper than walking D steps, an optimal route uses
     * the minimum possible number of jumps.
     */
    if (jump_delta >= 0) {
        vector<ReachInterval> reachable;

        auto add_component = [&](int64 s, int64 e) {
            if (s > e || s > max_query) return;
            e = min(e, max_query);

            if (s == 0) {
                reachable.push_back({0, e, 0});
                return;
            }

            int64 low = max<int64>(0, s - D);
            int64 high = e - D;
            if (low > high) return;

            int lo = 0, hi = (int)reachable.size();
            while (lo < hi) {
                int mid = (lo + hi) >> 1;
                if (reachable[mid].right >= low) hi = mid;
                else lo = mid + 1;
            }

            if (lo == (int)reachable.size()) return;

            const ReachInterval &src = reachable[lo];
            int64 takeoff = max(low, src.left);
            if (takeoff > high || takeoff > src.right) return;

            reachable.push_back({
                takeoff + D,
                e,
                src.jumps + 1
            });
        };

        int64 free_start = 0;
        for (int i = 0; i < N && free_start <= max_query; ++i) {
            if (L[i] > max_query) break;
            add_component(free_start, L[i] - 1);
            free_start = R[i] + 1;
        }
        add_component(free_start, max_query);

        for (int64 x : X) {
            int lo = 0, hi = (int)reachable.size();
            while (lo < hi) {
                int mid = (lo + hi) >> 1;
                if (reachable[mid].right >= x) hi = mid;
                else lo = mid + 1;
            }

            if (lo == (int)reachable.size() ||
                x < reachable[lo].left) {
                cout << -1 << '\n';
                continue;
            }

            int64 jumps = reachable[lo].jumps;
            i128 answer = (i128)A * x + jump_delta * jumps;
            print_i128(answer);
        }
        return 0;
    }

    /*
     * A construction containing at least D consecutive steps is impossible
     * to cross. Only coordinates before the first such construction matter.
     */
    int64 cutoff = max_query + 1;
    int relevant_obstacles = N;

    for (int i = 0; i < N; ++i) {
        if (R[i] - L[i] + 1 >= D) {
            cutoff = L[i];
            relevant_obstacles = i;
            break;
        }
    }

    vector<char> relevant_query(Q, false);
    int64 max_relevant_query = -1;
    for (int i = 0; i < Q; ++i) {
        if (X[i] < cutoff) {
            relevant_query[i] = true;
            max_relevant_query = max(max_relevant_query, X[i]);
        }
    }

    if (max_relevant_query < 0) {
        for (int i = 0; i < Q; ++i) cout << -1 << '\n';
        return 0;
    }

    map<int64, RowEvent> events;
    vector<int64> coordinates = {0, D};
    if (D > 1) coordinates.push_back(1);

    auto add_block = [&](int64 row, int64 l, int64 r) {
        if (l > r) return;
        events[row].blocked.push_back({l, r});
        coordinates.push_back(l);
        coordinates.push_back(r + 1);
    };

    for (int i = 0; i < relevant_obstacles; ++i) {
        if (L[i] > max_relevant_query) break;

        int64 l = L[i];
        int64 r = min(R[i], max_relevant_query);

        int64 ql = l / D;
        int64 qr = r / D;
        int64 rl = l % D;
        int64 rr = r % D;

        if (ql == qr) {
            add_block(ql, rl, rr);
        } else {
            add_block(ql, rl, D - 1);
            add_block(qr, 0, rr);
        }
    }

    for (int i = 0; i < Q; ++i) {
        if (relevant_query[i]) {
            events[X[i] / D].queries.push_back(i);
        }
    }

    events[0];

    sort(coordinates.begin(), coordinates.end());
    coordinates.erase(unique(coordinates.begin(), coordinates.end()),
                      coordinates.end());

    int atoms = (int)coordinates.size() - 1;
    SegmentTree seg(atoms);

    auto atom_at = [&](int64 residue) {
        return int(upper_bound(coordinates.begin(), coordinates.end(), residue)
                   - coordinates.begin()) - 1;
    };

    auto boundary_index = [&](int64 coordinate) {
        return int(lower_bound(coordinates.begin(), coordinates.end(), coordinate)
                   - coordinates.begin());
    };

    auto assign_residue_range = [&](int64 l, int64 r, int64 value) {
        if (l > r) return;
        int left_atom = boundary_index(l);
        int right_atom = boundary_index(r + 1) - 1;
        seg.assign_range(left_atom, right_atom, value);
    };

    auto prefix_residue_range = [&](int64 l, int64 r) {
        if (l > r) return;
        int left_atom = boundary_index(l);
        int right_atom = boundary_index(r + 1) - 1;
        seg.prefix_min(left_atom, right_atom);
    };

    auto merge_blocks = [&](vector<pair<int64, int64>> blocks) {
        sort(blocks.begin(), blocks.end());
        vector<pair<int64, int64>> merged;

        for (auto [l, r] : blocks) {
            if (merged.empty() || l > merged.back().second + 1) {
                merged.push_back({l, r});
            } else {
                merged.back().second = max(merged.back().second, r);
            }
        }
        return merged;
    };

    auto add_wrap_candidate = [&]() {
        int64 end_f = seg.get_point(atom_at(D - 1));
        if (end_f == INF) return;

        int64 candidate = end_f + D;
        int zero_atom = atom_at(0);
        int64 current = seg.get_point(zero_atom);
        if (candidate < current) {
            seg.assign_range(zero_atom, zero_atom, candidate);
        }
    };

    auto process_empty_row = [&]() {
        add_wrap_candidate();
        prefix_residue_range(0, D - 1);
    };

    auto process_blocked_row =
        [&](const vector<pair<int64, int64>> &raw_blocks, bool initial_row) {
            if (!initial_row) add_wrap_candidate();

            vector<pair<int64, int64>> blocks = merge_blocks(raw_blocks);
            for (auto [l, r] : blocks) {
                assign_residue_range(l, r, INF);
            }

            int64 start = 0;
            for (auto [l, r] : blocks) {
                if (start < l) prefix_residue_range(start, l - 1);
                start = r + 1;
            }
            if (start < D) prefix_residue_range(start, D - 1);
        };

    vector<i128> answers(Q, -1);

    int zero_atom = atom_at(0);
    seg.assign_range(zero_atom, zero_atom, 0);

    int64 previous_row = -1;

    for (auto &[row, event] : events) {
        if (row == 0) {
            process_blocked_row(event.blocked, true);
        } else {
            if (row - previous_row >= 2) {
                process_empty_row();
            }
            process_blocked_row(event.blocked, false);
        }

        for (int id : event.queries) {
            int64 residue = X[id] % D;
            int64 f = seg.get_point(atom_at(residue));

            if (f == INF) continue;

            int64 walks = f + residue;
            if (walks < 0 || walks > X[id] ||
                (X[id] - walks) % D != 0) {
                continue;
            }

            int64 jumps = (X[id] - walks) / D;
            answers[id] = (i128)A * walks + (i128)B * jumps;
        }

        previous_row = row;
    }

    for (int i = 0; i < Q; ++i) {
        if (!relevant_query[i] || answers[i] < 0) {
            cout << -1 << '\n';
        } else {
            print_i128(answers[i]);
        }
    }

    return 0;
}
