#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

using int64 = long long;

struct Student {
    int64 s, t, total;
};

struct Query {
    int64 x, y, z;
    int id;
};

class Fenwick2D {
private:
    int size;
    vector<int64> tValues;
    vector<vector<int64>> coordinates;
    vector<vector<int>> trees;

    int reversedIndex(int64 t) const {
        int ascendingIndex =
            static_cast<int>(lower_bound(tValues.begin(), tValues.end(), t)
                             - tValues.begin());
        return size - ascendingIndex;
    }

    static void innerAdd(vector<int>& tree, int index, int value) {
        for (int i = index; i < static_cast<int>(tree.size()); i += i & -i) {
            tree[i] += value;
        }
    }

    static int innerPrefixSum(const vector<int>& tree, int index) {
        int result = 0;
        for (int i = index; i > 0; i -= i & -i) {
            result += tree[i];
        }
        return result;
    }

public:
    explicit Fenwick2D(const vector<Student>& students) {
        tValues.reserve(students.size());
        for (const Student& student : students) {
            tValues.push_back(student.t);
        }

        sort(tValues.begin(), tValues.end());
        tValues.erase(unique(tValues.begin(), tValues.end()), tValues.end());

        size = static_cast<int>(tValues.size());
        coordinates.resize(size + 1);
        trees.resize(size + 1);

        for (const Student& student : students) {
            int index = reversedIndex(student.t);
            for (int i = index; i <= size; i += i & -i) {
                coordinates[i].push_back(student.total);
            }
        }

        for (int i = 1; i <= size; ++i) {
            sort(coordinates[i].begin(), coordinates[i].end());
            coordinates[i].erase(
                unique(coordinates[i].begin(), coordinates[i].end()),
                coordinates[i].end()
            );
            trees[i].assign(coordinates[i].size() + 1, 0);
        }
    }

    void add(int64 t, int64 total) {
        int index = reversedIndex(t);

        for (int i = index; i <= size; i += i & -i) {
            int innerIndex =
                static_cast<int>(
                    lower_bound(coordinates[i].begin(),
                                coordinates[i].end(),
                                total)
                    - coordinates[i].begin()
                ) + 1;

            innerAdd(trees[i], innerIndex, 1);
        }
    }

    int query(int64 minimumT, int64 minimumTotal) const {
        int index =
            static_cast<int>(
                tValues.end()
                - lower_bound(tValues.begin(), tValues.end(), minimumT)
            );

        int result = 0;

        for (int i = index; i > 0; i -= i & -i) {
            int belowCount =
                static_cast<int>(
                    lower_bound(coordinates[i].begin(),
                                coordinates[i].end(),
                                minimumTotal)
                    - coordinates[i].begin()
                );

            int totalActive =
                innerPrefixSum(trees[i],
                               static_cast<int>(coordinates[i].size()));
            int activeBelow = innerPrefixSum(trees[i], belowCount);

            result += totalActive - activeBelow;
        }

        return result;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, q;
    cin >> n >> q;

    vector<Student> students(n);
    for (Student& student : students) {
        cin >> student.s >> student.t;
        student.total = student.s + student.t;
    }

    vector<Query> queries(q);
    for (int i = 0; i < q; ++i) {
        cin >> queries[i].x >> queries[i].y >> queries[i].z;
        queries[i].id = i;
    }

    Fenwick2D dataStructure(students);

    sort(students.begin(), students.end(),
         [](const Student& a, const Student& b) {
             return a.s > b.s;
         });

    sort(queries.begin(), queries.end(),
         [](const Query& a, const Query& b) {
             return a.x > b.x;
         });

    vector<int> answers(q);
    int studentIndex = 0;

    for (const Query& query : queries) {
        while (studentIndex < n &&
               students[studentIndex].s >= query.x) {
            dataStructure.add(students[studentIndex].t,
                              students[studentIndex].total);
            ++studentIndex;
        }

        answers[query.id] = dataStructure.query(query.y, query.z);
    }

    for (int answer : answers) {
        cout << answer << '\n';
    }

    return 0;
}