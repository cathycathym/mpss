#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
const int64 INF = (1LL << 62);

struct Solver {
    vector<vector<int>> clauses;
    vector<vector<pair<int, bool>>> occurrences;
    vector<int> true_count, unassigned_count;
    vector<signed char> value;
    vector<int> trail;
    set<int> active;

    explicit Solver(int variables = 0) {
        occurrences.resize(variables);
        value.assign(variables, -1);
    }

    bool add_clause(vector<int> clause) {
        sort(clause.begin(), clause.end());
        clause.erase(unique(clause.begin(), clause.end()), clause.end());

        for (int lit : clause) {
            if (binary_search(clause.begin(), clause.end(), -lit))
                return true;
        }
        if (clause.empty())
            return false;

        int id = (int)clauses.size();
        clauses.push_back(move(clause));
        true_count.push_back(0);
        unassigned_count.push_back((int)clauses.back().size());
        active.insert(id);

        for (int lit : clauses.back()) {
            int variable = abs(lit) - 1;
            occurrences[variable].push_back({id, lit > 0});
        }
        return true;
    }

    void undo(size_t checkpoint) {
        while (trail.size() > checkpoint) {
            int variable = trail.back();
            trail.pop_back();
            bool assigned_value = value[variable];

            for (auto [clause_id, positive] : occurrences[variable]) {
                bool literal_true = (assigned_value == positive);
                if (literal_true) {
                    --true_count[clause_id];
                    if (true_count[clause_id] == 0)
                        active.insert(clause_id);
                }
                ++unassigned_count[clause_id];
            }
            value[variable] = -1;
        }
    }

    bool assign_variable(int variable, bool assigned_value,
                         vector<int>& unit_queue) {
        if (value[variable] != -1)
            return value[variable] == assigned_value;

        value[variable] = assigned_value;
        trail.push_back(variable);

        for (auto [clause_id, positive] : occurrences[variable]) {
            bool literal_true = (assigned_value == positive);
            --unassigned_count[clause_id];

            if (literal_true) {
                if (true_count[clause_id] == 0)
                    active.erase(clause_id);
                ++true_count[clause_id];
            } else if (true_count[clause_id] == 0) {
                if (unassigned_count[clause_id] == 0)
                    return false;
                if (unassigned_count[clause_id] == 1)
                    unit_queue.push_back(clause_id);
            }
        }
        return true;
    }

    bool propagate(vector<int>& unit_queue) {
        while (!unit_queue.empty()) {
            int clause_id = unit_queue.back();
            unit_queue.pop_back();

            if (true_count[clause_id] != 0)
                continue;

            int unit_literal = 0;
            int remaining = 0;
            for (int lit : clauses[clause_id]) {
                int variable = abs(lit) - 1;
                if (value[variable] == -1) {
                    unit_literal = lit;
                    ++remaining;
                }
            }

            if (remaining == 0)
                return false;
            if (remaining != 1)
                continue;

            int variable = abs(unit_literal) - 1;
            bool required_value = unit_literal > 0;
            if (!assign_variable(variable, required_value, unit_queue))
                return false;
        }
        return true;
    }

    bool dfs() {
        if (active.empty())
            return true;

        int clause_id = *active.begin();
        int chosen_variable = -1;
        bool preferred_value = false;
        size_t best_occurrences = 0;

        for (int lit : clauses[clause_id]) {
            int variable = abs(lit) - 1;
            if (value[variable] == -1 &&
                occurrences[variable].size() >= best_occurrences) {
                best_occurrences = occurrences[variable].size();
                chosen_variable = variable;
                preferred_value = lit > 0;
            }
        }

        if (chosen_variable == -1)
            return false;

        for (int attempt = 0; attempt < 2; ++attempt) {
            bool assigned_value =
                attempt == 0 ? preferred_value : !preferred_value;
            size_t checkpoint = trail.size();
            vector<int> unit_queue;

            if (assign_variable(chosen_variable, assigned_value, unit_queue) &&
                propagate(unit_queue) && dfs())
                return true;

            undo(checkpoint);
        }
        return false;
    }

    bool solve() {
        vector<int> unit_queue;
        for (int i = 0; i < (int)clauses.size(); ++i) {
            if (clauses[i].size() == 1)
                unit_queue.push_back(i);
        }
        return propagate(unit_queue) && dfs();
    }
};

pair<int64, string> solve_small(const vector<int64>& sprinklers,
                                const vector<int64>& flowers) {
    int n = (int)sprinklers.size();
    int total_masks = 1 << n;
    int64 best = INF;
    int best_mask = -1;

    for (int mask = 0; mask < total_masks; ++mask) {
        int64 needed = 0;
        bool possible = true;

        for (int64 flower : flowers) {
            int64 distance = INF;
            for (int i = 0; i < n; ++i) {
                bool right = (mask >> i) & 1;
                if ((right && sprinklers[i] <= flower) ||
                    (!right && sprinklers[i] >= flower)) {
                    distance = min(distance, llabs(sprinklers[i] - flower));
                }
            }
            if (distance == INF) {
                possible = false;
                break;
            }
            needed = max(needed, distance);
        }

        if (possible && needed < best) {
            best = needed;
            best_mask = mask;
        }
    }

    if (best_mask == -1)
        return {-1, ""};

    string answer(n, 'L');
    for (int i = 0; i < n; ++i)
        if ((best_mask >> i) & 1)
            answer[i] = 'R';
    return {best, answer};
}

bool grouped_in_triples(const vector<int64>& sprinklers) {
    if (sprinklers.size() % 3 != 0)
        return false;
    for (int i = 0; i < (int)sprinklers.size(); i += 3) {
        if (sprinklers[i] != sprinklers[i + 1] ||
            sprinklers[i] != sprinklers[i + 2])
            return false;
    }
    return true;
}

pair<int64, string> solve_triples(const vector<int64>& sprinklers,
                                  const vector<int64>& flowers) {
    int n = (int)sprinklers.size();
    int64 needed = 0;

    for (int64 flower : flowers) {
        auto it = lower_bound(sprinklers.begin(), sprinklers.end(), flower);
        int64 closest = INF;
        if (it != sprinklers.end())
            closest = min(closest, llabs(*it - flower));
        if (it != sprinklers.begin()) {
            --it;
            closest = min(closest, llabs(*it - flower));
        }
        needed = max(needed, closest);
    }

    string answer(n, 'R');
    for (int i = 0; i < n; i += 3) {
        answer[i] = 'L';
        answer[i + 1] = 'R';
        answer[i + 2] = 'R';
    }
    return {needed, answer};
}

bool solve_bounded_power(const vector<int64>& sprinklers,
                         const vector<int64>& flowers, int64 power,
                         string& answer) {
    int n = (int)sprinklers.size();

    vector<int64> singleton_positions;
    vector<int> singleton_original_index;
    vector<int64> double_positions;
    vector<vector<int>> groups;

    for (int i = 0; i < n;) {
        int j = i + 1;
        while (j < n && sprinklers[j] == sprinklers[i])
            ++j;

        vector<int> group;
        for (int k = i; k < j; ++k)
            group.push_back(k);
        groups.push_back(group);

        if (j - i == 1) {
            singleton_positions.push_back(sprinklers[i]);
            singleton_original_index.push_back(i);
        } else {
            double_positions.push_back(sprinklers[i]);
        }
        i = j;
    }

    Solver solver((int)singleton_positions.size());

    for (int64 flower : flowers) {
        if (binary_search(sprinklers.begin(), sprinklers.end(), flower))
            continue;

        auto multi_it = lower_bound(double_positions.begin(),
                                    double_positions.end(), flower - power);
        if (multi_it != double_positions.end() &&
            *multi_it <= flower + power)
            continue;

        vector<int> clause;
        auto left = lower_bound(singleton_positions.begin(),
                                singleton_positions.end(), flower - power);
        auto right = upper_bound(singleton_positions.begin(),
                                 singleton_positions.end(), flower + power);

        for (auto it = left; it != right; ++it) {
            int variable = (int)(it - singleton_positions.begin());
            if (*it < flower)
                clause.push_back(variable + 1);
            else if (*it > flower)
                clause.push_back(-(variable + 1));
        }

        if (!solver.add_clause(move(clause)))
            return false;
    }

    if (!solver.solve())
        return false;

    answer.assign(n, 'R');

    for (const auto& group : groups) {
        if (group.size() >= 2) {
            answer[group[0]] = 'L';
            for (size_t i = 1; i < group.size(); ++i)
                answer[group[i]] = 'R';
        }
    }

    for (int variable = 0;
         variable < (int)singleton_original_index.size(); ++variable) {
        int original = singleton_original_index[variable];
        answer[original] =
            solver.value[variable] == 0 ? 'L' : 'R';
    }
    return true;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    vector<int64> sprinklers(n), flowers(m);
    for (int64& position : sprinklers)
        cin >> position;
    for (int64& position : flowers)
        cin >> position;

    if (n <= 10) {
        auto result = solve_small(sprinklers, flowers);
        if (result.first < 0) {
            cout << -1 << '\n';
        } else {
            cout << result.first << '\n' << result.second << '\n';
        }
        return 0;
    }

    if (grouped_in_triples(sprinklers)) {
        auto result = solve_triples(sprinklers, flowers);
        cout << result.first << '\n' << result.second << '\n';
        return 0;
    }

    for (int64 power = 0; power <= 8; ++power) {
        string answer;
        if (solve_bounded_power(sprinklers, flowers, power, answer)) {
            cout << power << '\n' << answer << '\n';
            return 0;
        }
    }

    cout << -1 << '\n';
    return 0;
}
