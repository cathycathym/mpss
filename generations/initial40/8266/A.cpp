#include <bits/stdc++.h>
using namespace std;

using ld = long double;

struct Point {
    ld x, y;
};

struct Event {
    ld at;
    int delta;
    bool operator<(const Event& other) const {
        return at < other.at;
    }
};

static ld sqdist(const Point& a, const Point& b) {
    ld dx = a.x - b.x, dy = a.y - b.y;
    return dx * dx + dy * dy;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int k, n;
    long long si, ti;
    cin >> k >> n >> si >> ti;

    ld s = si, t = ti;
    vector<Point> p(n);
    for (auto& q : p) cin >> q.x >> q.y;

    const ld INF = 1e100L;
    const ld EPS = 1e-24L;
    ld answer = INF;

    auto countAt = [&](const Point& c, ld r) {
        int cnt = 0;
        ld rr = r * r;
        for (const auto& q : p) {
            if (sqdist(c, q) <= rr + 1e-12L * max((ld)1.0, rr))
                ++cnt;
        }
        return cnt;
    };

    // Centers at the origin and at every star.
    {
        vector<ld> d;
        for (const auto& q : p) d.push_back(sqrtl(sqdist(Point{0, 0}, q)));
        nth_element(d.begin(), d.begin() + k - 1, d.end());
        answer = min(answer, t * d[k - 1]);
    }

    for (int i = 0; i < n; ++i) {
        vector<ld> d;
        d.reserve(n);
        for (const auto& q : p) d.push_back(sqrtl(sqdist(p[i], q)));
        nth_element(d.begin(), d.begin() + k - 1, d.end());
        answer = min(answer, s * sqrtl(sqdist(p[i], Point{0, 0})) + t * d[k - 1]);
    }

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            ld dx = p[j].x - p[i].x;
            ld dy = p[j].y - p[i].y;
            ld len = hypotl(dx, dy);
            if (len < EPS) continue;

            // c(u) = midpoint + u * v, where v is a unit perpendicular vector.
            Point mid{(p[i].x + p[j].x) / 2, (p[i].y + p[j].y) / 2};
            Point v{-dy / len, dx / len};
            ld a = len / 2;

            // Minimize telescope cost along this perpendicular bisector.
            // The function is convex, so binary search its derivative.
            ld b = mid.x * v.x + mid.y * v.y;
            auto derivative = [&](ld u) {
                ld d0 = hypotl(mid.x + u * v.x, mid.y + u * v.y);
                ld d1 = hypotl(a, u);
                ld first = (d0 < EPS ? 0 : s * (u + b) / d0);
                ld second = (d1 < EPS ? 0 : t * u / d1);
                return first + second;
            };

            ld lo = -1e15L, hi = 1e15L;
            for (int it = 0; it < 100; ++it) {
                ld m = (lo + hi) / 2;
                if (derivative(m) < 0) lo = m;
                else hi = m;
            }
            ld bestU = (lo + hi) / 2;

            auto cost = [&](ld u) {
                Point c{mid.x + u * v.x, mid.y + u * v.y};
                return s * hypotl(c.x, c.y) + t * hypotl(a, u);
            };

            vector<Event> events;
            events.reserve(n);
            int current = 0; // Number covered just to the right of -infinity.

            for (int z = 0; z < n; ++z) {
                ld mx = mid.x - p[z].x;
                ld my = mid.y - p[z].y;
                ld B = mx * mx + my * my - a * a;
                ld A = 2 * (v.x * mx + v.y * my);

                if (fabsl(A) < EPS) {
                    if (B <= 1e-10L) ++current;
                } else {
                    ld where = -B / A;
                    if (A > 0) {
                        // Covered for u <= where.
                        ++current;
                        events.push_back({where, -1});
                    } else {
                        // Covered for u >= where.
                        events.push_back({where, +1});
                    }
                }
            }

            sort(events.begin(), events.end());

            auto evaluateInterval = [&](ld left, ld right, int cnt) {
                if (cnt < k) return;
                ld u = bestU;
                if (u < left) u = left;
                if (u > right) u = right;
                answer = min(answer, cost(u));
            };

            ld previous = -INF;
            int pos = 0;
            while (pos < (int)events.size()) {
                ld x = events[pos].at;

                // Open interval before x, with endpoints valid for the stars
                // that are already covered there.
                evaluateInterval(previous, x, current);

                int end = pos;
                int additions = 0;
                while (end < (int)events.size() &&
                       fabsl(events[end].at - x) <= 1e-18L * max((ld)1.0, fabsl(x))) {
                    if (events[end].delta > 0) additions += events[end].delta;
                    ++end;
                }

                // At an event, both stars leaving and stars entering are covered.
                if (current + additions >= k)
                    answer = min(answer, cost(x));

                for (int q = pos; q < end; ++q) current += events[q].delta;
                previous = x;
                pos = end;
            }

            evaluateInterval(previous, INF, current);
        }
    }

    cout << fixed << setprecision(15) << (double)answer << '\n';
    return 0;
}
