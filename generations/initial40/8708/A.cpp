#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
using i128 = __int128_t;

struct Vec {
    int64 x, y;
};

int64 gcdll(int64 a, int64 b) {
    a = llabs(a);
    b = llabs(b);
    while (b != 0) {
        int64 r = a % b;
        a = b;
        b = r;
    }
    return a;
}

// Returns gcd(a,b), and coefficients x,y such that ax+by=gcd(a,b).
int64 exgcd(int64 a, int64 b, int64 &x, int64 &y) {
    if (b == 0) {
        x = (a >= 0 ? 1 : -1);
        y = 0;
        return llabs(a);
    }
    int64 x1, y1;
    int64 g = exgcd(b, a % b, x1, y1);
    x = y1;
    y = x1 - (a / b) * y1;
    return g;
}

int64 gcd128_with_ll(i128 a, int64 b) {
    if (a < 0) a = -a;
    while (b != 0) {
        int64 r = (int64)(a % b);
        a = b;
        b = r;
    }
    return (int64)a;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<Vec> p(n);
    for (auto &v : p) cin >> v.x >> v.y;

    // rank = 0: no nonzero vector
    // rank = 1: b1 generates the one-dimensional lattice
    // rank = 2: basis is (a,0), (b,c), in column Hermite form.
    int rank = 0;
    Vec b1{0, 0};
    int64 a = 0, b = 0, c = 0;

    for (int i = 1; i < n; ++i) {
        Vec v{p[i].x - p[0].x, p[i].y - p[0].y};
        if (v.x == 0 && v.y == 0) continue;

        if (rank == 0) {
            b1 = v;
            rank = 1;
            continue;
        }

        if (rank == 1) {
            i128 det = (i128)b1.x * v.y - (i128)b1.y * v.x;

            if (det == 0) {
                // Merge two collinear generators into one.
                int64 g1 = gcdll(b1.x, b1.y);
                Vec primitive{b1.x / g1, b1.y / g1};

                int64 k1, k2;
                if (primitive.x != 0) {
                    k1 = b1.x / primitive.x;
                    k2 = v.x / primitive.x;
                } else {
                    k1 = b1.y / primitive.y;
                    k2 = v.y / primitive.y;
                }

                int64 g = gcdll(k1, k2);
                b1 = {primitive.x * g, primitive.y * g};
            } else {
                // Convert the two generators to Hermite normal form:
                // (a,0), (b,c).
                int64 s, t;
                c = exgcd(b1.y, v.y, s, t);
                i128 bx = (i128)s * b1.x + (i128)t * v.x;

                i128 d128 = det < 0 ? -det : det;
                int64 d = (int64)d128;
                a = d / c;

                b = (int64)(bx % a);
                if (b < 0) b += a;

                rank = 2;
            }
            continue;
        }

        // Current basis: (a,0), (b,c).
        // The new index is gcd of all 2x2 minors.
        i128 det2 = (i128)b * v.y - (i128)c * v.x;
        int64 d = gcdll(a * c, a * v.y);
        d = gcdll(d, (int64)(det2 < 0 ? -det2 : det2));

        // New gcd of y-coordinates and a corresponding x-coordinate.
        int64 s, t;
        int64 newC = exgcd(c, v.y, s, t);
        i128 newB128 = (i128)s * b + (i128)t * v.x;

        int64 newA = d / newC;
        int64 newB = (int64)(newB128 % newA);
        if (newB < 0) newB += newA;

        a = newA;
        b = newB;
        c = newC;
    }

    if (rank < 2) {
        cout << -1 << '\n';
    } else {
        cout << a * c << '\n';
    }

    return 0;
}
