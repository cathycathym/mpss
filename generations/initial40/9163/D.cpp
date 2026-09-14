#include <bits/stdc++.h>
using namespace std;

using ll = long long;
const ll INF = (1LL << 62);

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, sl, el;
    ll sc, ec;

    cin >> n;
    cin >> sl >> sc;
    cin >> el >> ec;

    --sl;
    --el;
    --sc;
    --ec;

    vector<ll> len(n);
    for (ll &x : len) cin >> x;

    // Vertex 2*i is the start of line i, and 2*i+1 is its end.
    vector<ll> dist(2 * n, INF);
    vector<ll> tailToTarget(n);

    priority_queue<pair<ll, int>,
                   vector<pair<ll, int>>,
                   greater<pair<ll, int>>> pq;

    // Direct paths from the source to every line start.
    for (int i = 0; i < n; ++i) {
        dist[2 * i] = (ll)abs(i - sl) + sc;
    }

    // Direct paths from the source to every line end.
    ll mn = sc;
    for (int i = sl; i >= 0; --i) {
        mn = min(mn, len[i]);
        dist[2 * i + 1] =
            min(dist[2 * i + 1], (ll)(sl - i) + len[i] - mn);
    }

    mn = sc;
    for (int i = sl; i < n; ++i) {
        mn = min(mn, len[i]);
        dist[2 * i + 1] =
            min(dist[2 * i + 1], (ll)(i - sl) + len[i] - mn);
    }

    // Direct paths from every line end to the target.
    mn = INF;
    for (int i = el; i >= 0; --i) {
        mn = min(mn, len[i]);
        tailToTarget[i] = (ll)(el - i) + llabs(mn - ec);
    }

    mn = INF;
    for (int i = el; i < n; ++i) {
        mn = min(mn, len[i]);
        tailToTarget[i] = (ll)(i - el) + llabs(mn - ec);
    }

    ll answer = INF;

    // A boundary-free Manhattan path is possible at this column.
    ll requiredColumn = min(sc, ec);
    bool manhattanPossible = true;
    for (int i = min(sl, el); i <= max(sl, el); ++i) {
        if (len[i] < requiredColumn) {
            manhattanPossible = false;
            break;
        }
    }
    if (manhattanPossible) {
        answer = (ll)abs(sl - el) + llabs(sc - ec);
    }

    for (int v = 0; v < 2 * n; ++v) {
        pq.push({dist[v], v});
    }

    auto relax = [&](int v, ll nd) {
        if (nd < dist[v]) {
            dist[v] = nd;
            pq.push({nd, v});
        }
    };

    while (!pq.empty()) {
        ll d = pq.top().first;
        int v = pq.top().second;
        pq.pop();

        if (d != dist[v]) continue;
        if (d >= answer) break;

        int line = v / 2;
        bool isTail = (v & 1);

        if (isTail) {
            answer = min(answer, d + tailToTarget[line]);

            // Move horizontally to the start of this line.
            relax(2 * line, d + len[line]);

            // Right at the line end enters the next line.
            if (line + 1 < n) {
                relax(2 * (line + 1), d + 1);
            }
        } else {
            answer = min(answer, d + (ll)abs(line - el) + ec);

            // Move horizontally to the end of this line.
            relax(2 * line + 1, d + len[line]);

            // Vertical moves at column zero.
            if (line > 0) {
                relax(2 * (line - 1), d + 1);
            }
            if (line + 1 < n) {
                relax(2 * (line + 1), d + 1);
            }

            // Left at the line start enters the previous line's end.
            if (line > 0) {
                relax(2 * (line - 1) + 1, d + 1);
            }
        }
    }

    cout << answer << '\n';
    return 0;
}