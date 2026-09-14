#include <algorithm>
#include <iostream>
#include <vector>

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
    int size;
    vector<int64> tValues;
    vector<vector<int64>> totalCoordinates;
    vector<vector<int>> trees;

    int reversedTIndex(int64 t) const {
        int pos = lower_bound(tValues.begin(), tValues.end(), t) - tValues.begin();
        return size - pos;
    }

    static void addToTree(vector<int>& tree, int index) {
        for (int i = index; i < static_cast<int>(tree.size()); i += i & -i) {
            ++tree[i];
        }
    }

    static int prefixSum(const vector<int>& tree, int index) {
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
        totalCoordinates.resize(size + 1);
        trees.resize(size + 1);

        for (const Student& student : students) {
            int index = reversedTIndex(student.t);
            for (int i = index; i <= size; i += i & -i) {
                totalCoordinates[i].push_back(student.total);
            }
        }

        for (int i = 1; i <= size; ++i) {
            auto& coordinates = totalCoordinates[i];
            sort(coordinates.begin(), coordinates.end());
            coordinates.erase(unique(coordinates.begin(), coordinates.end()),
                              coordinates.end());
            trees[i].assign(coordinates.size() + 1, 0);
        }
    }

    void add(int64 t, int64 total) {
        int index = reversedTIndex(t);

        for (int i = index; i <= size; i += i & -i) {
            int totalIndex =
                lower_bound(totalCoordinates[i].begin(),
                            totalCoordinates[i].end(),
                            total) -
                totalCoordinates[i].begin() + 1;

            addToTree(trees[i], totalIndex);
        }
    }

    int countAtLeast(int64 minimumT, int64 minimumTotal) const {
        int outerIndex =
            tValues.end() -
            lower_bound(tValues.begin(), tValues.end(), minimumT);

        int result = 0;

        for (int i = outerIndex; i > 0; i -= i & -i) {
            int belowIndex =
                lower_bound(totalCoordinates[i].begin(),
                            totalCoordinates[i].end(),
                            minimumTotal) -
                totalCoordinates[i].begin();

            int allActive =
                prefixSum(trees[i], static_cast<int>(totalCoordinates[i].size()));
            int belowMinimum = prefixSum(trees[i], belowIndex);

            result += allActive - belowMinimum;
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
    int nextStudent = 0;

    for (const Query& query : queries) {
        while (nextStudent < n && students[nextStudent].s >= query.x) {
            dataStructure.add(students[nextStudent].t,
                              students[nextStudent].total);
            ++nextStudent;
        }

        answers[query.id] =
            dataStructure.countAtLeast(query.y, query.z);
    }

    for (int answer : answers) {
        cout << answer << '\n';
    }

    return 0;
}