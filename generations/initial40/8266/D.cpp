#include <bits/stdc++.h>
using namespace std;

using ld = long double;
using i128 = __int128_t;

struct Point {
    ld x, y;
};

struct Event {
    ld u;
    int delta;

    bool operator<(const Event& other) const {
        return u < other.u;
    }
};

static ld distanceTo(const Point& a, const Point& b) {
    return hypotl(a.x - b.x, a.y - b.y);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int k, n;
    long long ss, tt;
    cin >> k >> n >> ss >> tt;

    const ld s = (ld)ss;
    const ld t = (ld)tt;

    vector<long long> xi(n), yi(n);
    vector<Point> p(n);

    for (int i = 0; i < n; ++i) {
        cin >> xi[i] >> yi[i];
        p[i] = {(ld)xi[i], (ld)yi[i]};
    }

    vector<ld> distances(n);

    for (int i = 0; i < n; ++i)
        distances[i] = hypotl(p[i].x, p[i].y);

    nth_element(distances.begin(), distances.begin() + k - 1, distances.end());
    ld answer = t * distances[k - 1];

    if (t <= s) {
        cout << fixed << setprecision(15) << answer << '\n';
        return 0;
    }

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j)
            distances[j] = distanceTo(p[i], p[j]);

        nth_element(distances.begin(), distances.begin() + k - 1, distances.end());
        answer = min(answer, s * hypotl(p[i].x, p[i].y)
                           + t * distances[k - 1]);
    }

    if (answer == 0) {
        cout << fixed << setprecision(15) << 0.0L << '\n';
        return 0;
    }

    const ld INF = 1e100L;

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            long long dxInt = xi[j] - xi[i];
            long long dyInt = yi[j] - yi[i];

            if (dxInt == 0 && dyInt == 0)
                continue;

            ld dx = (ld)dxInt;
            ld dy = (ld)dyInt;
            ld length = hypotl(dx, dy);
            ld halfLength = length * 0.5L;

            Point mid{
                (p[i].x + p[j].x) * 0.5L,
                (p[i].y + p[j].y) * 0.5L
            };
            Point perpendicular{-dy / length, dx / length};

            ld projection = mid.x * perpendicular.x
                          + mid.y * perpendicular.y;

            auto derivative = [&](ld u) {
                ld cx = mid.x + u * perpendicular.x;
                ld cy = mid.y + u * perpendicular.y;
                ld fromOrigin = hypotl(cx, cy);
                ld radius = hypotl(halfLength, u);

                ld result = t * u / radius;
                if (fromOrigin != 0)
                    result += s * (u + projection) / fromOrigin;
                return result;
            };

            ld lo = -1.0L;
            ld hi = 1.0L;

            for (int step = 0; step < 256 && derivative(lo) > 0; ++step)
                lo *= 2.0L;
            for (int step = 0; step < 256 && derivative(hi) < 0; ++step)
                hi *= 2.0L;

            for (int iteration = 0; iteration < 100; ++iteration) {
                ld middle = (lo + hi) * 0.5L;
                if (derivative(middle) < 0)
                    lo = middle;
                else
                    hi = middle;
            }

            ld bestU = (lo + hi) * 0.5L;

            auto cost = [&](ld u) {
                ld cx = mid.x + u * perpendicular.x;
                ld cy = mid.y + u * perpendicular.y;
                return s * hypotl(cx, cy) + t * hypotl(halfLength, u);
            };

            ld minimumPairCost = cost(bestU);
            ld pruningMargin = 1e-15L * max((ld)1.0L, fabsl(answer));
            if (minimumPairCost > answer + pruningMargin)
                continue;

            vector<Event> events;
            events.reserve(n);
            int coveredAtNegativeInfinity = 0;
            int coveredAtBest = 0;

            i128 squaredLength =
                (i128)dxInt * dxInt + (i128)dyInt * dyInt;

            for (int z = 0; z < n; ++z) {
                i128 twiceX = (i128)xi[i] + xi[j] - (i128)2 * xi[z];
                i128 twiceY = (i128)yi[i] + yi[j] - (i128)2 * yi[z];

                i128 numerator =
                    -(i128)dyInt * twiceX + (i128)dxInt * twiceY;
                i128 constant =
                    twiceX * twiceX + twiceY * twiceY - squaredLength;

                if (numerator == 0) {
                    if (constant <= 0) {
                        ++coveredAtNegativeInfinity;
                        ++coveredAtBest;
                    }
                    continue;
                }

                ld eventU =
                    -(ld)constant * length / ((ld)4.0L * (ld)numerator);

                if (numerator > 0) {
                    ++coveredAtNegativeInfinity;
                    events.push_back({eventU, -1});
                    if (bestU <= eventU)
                        ++coveredAtBest;
                } else {
                    events.push_back({eventU, +1});
                    if (bestU >= eventU)
                        ++coveredAtBest;
                }
            }

            if (coveredAtBest >= k) {
                answer = min(answer, minimumPairCost);
                continue;
            }

            sort(events.begin(), events.end());

            auto evaluateInterval = [&](ld left, ld right, int covered) {
                if (covered < k)
                    return;

                ld u = bestU;
                if (u < left) u = left;
                if (u > right) u = right;
                answer = min(answer, cost(u));
            };

            int covered = coveredAtNegativeInfinity;
            ld previous = -INF;

            for (int pos = 0; pos < (int)events.size();) {
                ld eventU = events[pos].u;
                evaluateInterval(previous, eventU, covered);

                int end = pos;
                int additions = 0;
                ld groupingTolerance =
                    1e-18L * max((ld)1.0L, fabsl(eventU));

                while (end < (int)events.size() &&
                       fabsl(events[end].u - eventU) <= groupingTolerance) {
                    if (events[end].delta > 0)
                        additions += events[end].delta;
                    ++end;
                }

                if (covered + additions >= k)
                    answer = min(answer, cost(eventU));

                for (int q = pos; q < end; ++q)
                    covered += events[q].delta;

                previous = eventU;
                pos = end;
            }

            evaluateInterval(previous, INF, covered);
        }
    }

    cout << fixed << setprecision(15) << answer << '\n';
    return 0;
}