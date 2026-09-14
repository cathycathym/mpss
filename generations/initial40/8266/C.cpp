#include <bits/stdc++.h>
using namespace std;

using ld = long double;
const ld PI = acosl(-1.0L);
const ld EPS = 1e-12L;

struct Point {
    ld x, y;
    Point() : x(0), y(0) {}
    Point(ld x_, ld y_) : x(x_), y(y_) {}
    Point operator + (const Point& o) const { return {x + o.x, y + o.y}; }
    Point operator - (const Point& o) const { return {x - o.x, y - o.y}; }
    Point operator * (ld k) const { return {x * k, y * k}; }
};

ld dot(const Point& a, const Point& b) {
    return a.x * b.x + a.y * b.y;
}
ld norm(const Point& a) {
    return sqrtl(dot(a, a));
}
ld dist(const Point& a, const Point& b) {
    return norm(a - b);
}

int k, n;
ld s, t;
vector<Point> p;

bool feasibleRadius(ld R) {
    if (R < 0) return false;

    for (int i = 0; i < n; ++i) {
        vector<pair<ld, int>> ev;
        int base = 1; // star i itself

        for (int j = 0; j < n; ++j) {
            if (i == j) continue;
            ld d = dist(p[i], p[j]);
            if (d < EPS) {
                ++base;
                continue;
            }
            if (d > 2 * R + EPS) continue;

            Point v = p[j] - p[i];
            ld a = atan2l(v.y, v.x);
            ld q = acosl(max((ld)-1, min((ld)1, d / (2 * R))));

            ld l = a - q, r = a + q;
            while (l < 0) l += 2 * PI;
            while (l >= 2 * PI) l -= 2 * PI;
            while (r < 0) r += 2 * PI;
            while (r >= 2 * PI) r -= 2 * PI;

            if (l <= r) {
                ev.push_back({l, +1});
                ev.push_back({r, -1});
            } else {
                ++base;
                ev.push_back({l, +1});
                ev.push_back({r, -1});
            }
        }

        if (base >= k) return true;
        sort(ev.begin(), ev.end());

        int cur = base;
        for (int z = 0; z < (int)ev.size();) {
            int w = z;
            int add = 0;
            while (w < (int)ev.size() && fabsl(ev[w].first - ev[z].first) < EPS) {
                add += ev[w].second;
                ++w;
            }
            cur += add;
            if (cur >= k) return true;
            z = w;
        }
    }
    return false;
}

struct Event {
    ld ang;
    int id;
    Point x;
    bool operator < (const Event& o) const {
        return ang < o.ang;
    }
};

bool feasibleCost(ld C) {
    int atOrigin = 0;
    for (int i = 0; i < n; ++i) {
        if (t * norm(p[i]) <= C + EPS) ++atOrigin;
    }
    if (atOrigin >= k) return true;

    // Degenerate ovals: their only possible point is p[i].
    for (int i = 0; i < n; ++i) {
        if (s * norm(p[i]) > C + EPS) continue;
        int cnt = 0;
        for (int j = 0; j < n; ++j) {
            if (s * norm(p[i]) + t * dist(p[i], p[j]) <= C + EPS) ++cnt;
        }
        if (cnt >= k) return true;
    }

    for (int i = 0; i < n; ++i) {
        if (s * norm(p[i]) >= C - EPS) continue;

        vector<Event> ev;

        for (int j = 0; j < n; ++j) {
            if (i == j) continue;

            Point d = p[j] - p[i];
            ld dd = norm(d);
            if (dd < EPS) continue;

            // Perpendicular bisector of p[i], p[j]:
            Point mid = (p[i] + p[j]) * 0.5L;
            Point u(-d.y / dd, d.x / dd);

            auto value = [&](ld z) {
                Point x = mid + u * z;
                return s * norm(x) + t * dist(x, p[i]);
            };

            // Every boundary point has |x| <= C/s.
            ld B = C / s + norm(mid) + 2.0L;

            ld lo = -B, hi = B;
            for (int it = 0; it < 45; ++it) {
                ld m1 = (2 * lo + hi) / 3;
                ld m2 = (lo + 2 * hi) / 3;
                if (value(m1) < value(m2)) hi = m2;
                else lo = m1;
            }
            ld z0 = (lo + hi) * 0.5L;

            if (value(z0) > C + 1e-9L) continue;

            auto rootLeft = [&]() {
                ld a = -B, b = z0;
                for (int it = 0; it < 55; ++it) {
                    ld m = (a + b) * 0.5L;
                    if (value(m) > C) a = m;
                    else b = m;
                }
                return (a + b) * 0.5L;
            };
            auto rootRight = [&]() {
                ld a = z0, b = B;
                for (int it = 0; it < 55; ++it) {
                    ld m = (a + b) * 0.5L;
                    if (value(m) > C) b = m;
                    else a = m;
                }
                return (a + b) * 0.5L;
            };

            ld zl = rootLeft(), zr = rootRight();
            Point x1 = mid + u * zl;
            Point x2 = mid + u * zr;

            ev.push_back({atan2l(x1.y - p[i].y, x1.x - p[i].x), j, x1});
            if (norm(x2 - x1) > 1e-8L)
                ev.push_back({atan2l(x2.y - p[i].y, x2.x - p[i].x), j, x2});
        }

        if (ev.empty()) continue;
        for (auto& e : ev) if (e.ang < 0) e.ang += 2 * PI;
        sort(ev.begin(), ev.end());

        vector<char> inside(n, 0);
        int cnt = 0;

        // Initialize directly at the first event point.
        Point x = ev[0].x;
        for (int q = 0; q < n; ++q) {
            inside[q] = (dist(x, p[q]) <= dist(x, p[i]) + 1e-8L);
            cnt += inside[q];
        }

        for (int z = 0; z < (int)ev.size();) {
            int w = z;
            while (w < (int)ev.size() && fabsl(ev[w].ang - ev[z].ang) < 1e-10L) ++w;

            for (int h = z; h < w; ++h) {
                int q = ev[h].id;
                Point radial = ev[h].x - p[i];
                Point tangent(-radial.y, radial.x);

                // Sign immediately after this event while moving counter-clockwise.
                ld deriv = 2 * dot(p[i] - p[q], tangent);
                if (fabsl(deriv) < 1e-10L) continue;

                bool nextInside = (deriv < 0);
                if (inside[q] != nextInside) {
                    inside[q] = nextInside;
                    cnt += nextInside ? 1 : -1;
                }
            }

            if (cnt >= k) return true;
            z = w;
        }
    }

    return false;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    long long ss, tt;
    cin >> k >> n >> ss >> tt;
    s = ss;
    t = tt;
    p.resize(n);

    for (int i = 0; i < n; ++i) {
        long long x, y;
        cin >> x >> y;
        p[i] = Point((ld)x, (ld)y);
    }

    vector<ld> originDistances(n);
    for (int i = 0; i < n; ++i) originDistances[i] = norm(p[i]);
    nth_element(originDistances.begin(), originDistances.begin() + k - 1, originDistances.end());
    ld originAnswer = t * originDistances[k - 1];

    if (t <= s) {
        cout << fixed << setprecision(15) << (double)originAnswer << '\n';
        return 0;
    }

    if (s == 0) {
        ld lo = 0, hi = originDistances[k - 1];
        for (int it = 0; it < 65; ++it) {
            ld mid = (lo + hi) * 0.5L;
            if (feasibleRadius(mid)) hi = mid;
            else lo = mid;
        }
        cout << fixed << setprecision(15) << (double)(t * hi) << '\n';
        return 0;
    }

    ld lo = 0, hi = originAnswer;
    for (int it = 0; it < 65; ++it) {
        ld mid = (lo + hi) * 0.5L;
        if (feasibleCost(mid)) hi = mid;
        else lo = mid;
    }

    cout << fixed << setprecision(15) << (double)hi << '\n';
    return 0;
}
