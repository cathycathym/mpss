#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
using u64 = uint64_t;

struct Key {
    u64 h1, h2;
    int size;

    bool operator==(const Key& other) const {
        return h1 == other.h1 && h2 == other.h2 && size == other.size;
    }
};

struct KeyHasher {
    size_t operator()(const Key& key) const {
        u64 x = key.h1 ^ (key.h2 + 0x9e3779b97f4a7c15ULL
                          + (key.h1 << 6) + (key.h1 >> 2));
        x ^= static_cast<u64>(key.size) * 0xbf58476d1ce4e5b9ULL;
        x ^= x >> 30;
        x *= 0xbf58476d1ce4e5b9ULL;
        x ^= x >> 27;
        x *= 0x94d049bb133111ebULL;
        x ^= x >> 31;
        return static_cast<size_t>(x);
    }
};

struct NumberSet {
    vector<int> values;
    u64 h1 = 0, h2 = 0;
    const vector<u64>* seed1;
    const vector<u64>* seed2;

    int insertValue(int x) {
        int pos = static_cast<int>(values.size());
        while (pos > 0 && values[pos - 1] > x) --pos;

        if (pos > 0 && values[pos - 1] == x) return -1;

        values.insert(values.begin() + pos, x);
        h1 ^= (*seed1)[x];
        h2 ^= (*seed2)[x];
        return pos;
    }

    void eraseAt(int pos) {
        if (pos == -1) return;
        int x = values[pos];
        h1 ^= (*seed1)[x];
        h2 ^= (*seed2)[x];
        values.erase(values.begin() + pos);
    }

    void appendLargest(int x) {
        values.push_back(x);
        h1 ^= (*seed1)[x];
        h2 ^= (*seed2)[x];
    }

    int removeLargest() {
        int x = values.back();
        values.pop_back();
        h1 ^= (*seed1)[x];
        h2 ^= (*seed2)[x];
        return x;
    }

    Key key() const {
        return {h1, h2, static_cast<int>(values.size())};
    }
};

static const int64 INF = (1LL << 62);

int n;
vector<int64> weightValue;
vector<vector<pair<int, int>>> factors;
unordered_map<Key, int64, KeyHasher> memo;

int64 solve(NumberSet& required) {
    if (required.values.size() == 1) {
        return weightValue[1];
    }

    Key currentKey = required.key();
    auto found = memo.find(currentKey);
    if (found != memo.end()) return found->second;

    int largest = required.removeLargest();
    int64 best = INF;

    int predecessorPosition = required.insertValue(largest - 1);
    best = min(best, solve(required));
    required.eraseAt(predecessorPosition);

    for (const auto& factorization : factors[largest]) {
        int a = factorization.first;
        int b = factorization.second;

        int positionA = required.insertValue(a);
        int positionB = required.insertValue(b);

        best = min(best, solve(required));

        required.eraseAt(positionB);
        required.eraseAt(positionA);
    }

    required.appendLargest(largest);
    int64 answer = best + weightValue[largest];
    memo.emplace(currentKey, answer);
    return answer;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n;

    weightValue.resize(n + 1);
    for (int i = 1; i <= n; ++i) {
        cin >> weightValue[i];
    }

    factors.resize(n + 1);
    for (int a = 2; a * a <= n; ++a) {
        for (int b = a; a * b <= n; ++b) {
            factors[a * b].push_back({a, b});
        }
    }

    vector<u64> seed1(n + 1), seed2(n + 1);
    mt19937_64 rng(
        chrono::steady_clock::now().time_since_epoch().count()
        ^ reinterpret_cast<uintptr_t>(&rng)
    );

    for (int i = 1; i <= n; ++i) {
        seed1[i] = rng();
        seed2[i] = rng();
    }

    memo.max_load_factor(0.7f);
    memo.reserve(1 << 18);

    NumberSet required;
    required.seed1 = &seed1;
    required.seed2 = &seed2;
    required.appendLargest(1);

    for (int value = 1; value <= n; ++value) {
        int position = required.insertValue(value);
        cout << solve(required) << '\n';
        required.eraseAt(position);
    }

    return 0;
}