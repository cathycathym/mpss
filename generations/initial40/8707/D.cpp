#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

class LeftistHeap {
    const vector<int64>& need;
    vector<int>& leftChild;
    vector<int>& rightChild;
    vector<int>& rankValue;

    bool before(int a, int b) const {
        if (need[a] != need[b]) return need[a] < need[b];
        return a < b;
    }

public:
    LeftistHeap(const vector<int64>& need_,
                vector<int>& leftChild_,
                vector<int>& rightChild_,
                vector<int>& rankValue_)
        : need(need_),
          leftChild(leftChild_),
          rightChild(rightChild_),
          rankValue(rankValue_) {}

    int meld(int a, int b) {
        if (a == 0) return b;
        if (b == 0) return a;

        if (!before(a, b)) swap(a, b);

        rightChild[a] = meld(rightChild[a], b);

        if (rankValue[leftChild[a]] < rankValue[rightChild[a]]) {
            swap(leftChild[a], rightChild[a]);
        }

        rankValue[a] = rankValue[rightChild[a]] + 1;
        return a;
    }

    int pop(int root) {
        int result = meld(leftChild[root], rightChild[root]);
        leftChild[root] = rightChild[root] = 0;
        rankValue[root] = 1;
        return result;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    int64 initialMoney;
    cin >> n >> initialMoney;

    vector<int64> profit(n + 1);
    vector<vector<int>> children(n + 1);
    vector<int> roots;

    for (int i = 1; i <= n; ++i) {
        int parent;
        cin >> profit[i] >> parent;

        if (parent == 0) roots.push_back(i);
        else children[parent].push_back(i);
    }

    // Every surviving package has positive total gain.
    // need[v] is the minimum capital required to execute package v.
    // successorHeap[v] contains the packages made available afterwards.
    vector<int64> gain(n + 1, 0);
    vector<int64> need(n + 1, 0);
    vector<int> successorHeap(n + 1, 0);
    vector<int> subtreeHeap(n + 1, 0);

    vector<int> leftChild(n + 1, 0);
    vector<int> rightChild(n + 1, 0);
    vector<int> rankValue(n + 1, 0);

    LeftistHeap heap(need, leftChild, rightChild, rankValue);

    for (int v = n; v >= 1; --v) {
        int available = 0;
        for (int u : children[v]) {
            available = heap.meld(available, subtreeHeap[u]);
        }

        if (profit[v] > 0) {
            gain[v] = profit[v];
            need[v] = 0;
            successorHeap[v] = available;
            rankValue[v] = 1;
            subtreeHeap[v] = v;
        } else if (profit[v] == 0) {
            // A zero-profit job can be completed for free, so its available
            // descendant packages can be promoted directly.
            subtreeHeap[v] = available;
        } else {
            int64 currentGain = profit[v];
            int64 required = -profit[v];

            // Merge the cheapest currently available positive packages until
            // the combined package itself has positive gain.
            while (currentGain <= 0 && available != 0) {
                int package = available;
                available = heap.pop(available);

                required = max(required, need[package] - currentGain);
                currentGain += gain[package];

                available = heap.meld(available, successorHeap[package]);
            }

            if (currentGain > 0) {
                gain[v] = currentGain;
                need[v] = required;
                successorHeap[v] = available;
                rankValue[v] = 1;
                subtreeHeap[v] = v;
            }
        }
    }

    int available = 0;
    for (int root : roots) {
        available = heap.meld(available, subtreeHeap[root]);
    }

    int64 money = initialMoney;

    while (available != 0) {
        int package = available;

        if (need[package] > money) break;

        available = heap.pop(available);
        money += gain[package];
        available = heap.meld(available, successorHeap[package]);
    }

    cout << money - initialMoney << '\n';
    return 0;
}