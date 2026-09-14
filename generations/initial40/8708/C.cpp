#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
using i128 = __int128_t;

int64 extended_gcd_positive(int64 a, int64 b, int64 &x, int64 &y) {
    if (b == 0) {
        x = 1;
        y = 0;
        return a;
    }
    int64 x1, y1;
    int64 g = extended_gcd_positive(b, a % b, x1, y1);
    x = y1;
    y = x1 - (a / b) * y1;
    return g;
}

// Returns g = gcd(|a|, |b|), with s*a + t*b = g.
int64 extended_gcd_signed(int64 a, int64 b, int64 &s, int64 &t) {
    if (a == 0 && b == 0) {
        s = t = 0;
        return 0;
    }
    if (a == 0) {
        s = 0;
        t = (b > 0 ? 1 : -1);
        return llabs(b);
    }
    if (b == 0) {
        s = (a > 0 ? 1 : -1);
        t = 0;
        return llabs(a);
    }

    int64 u, v;
    int64 g = extended_gcd_positive(llabs(a), llabs(b), u, v);
    s = (a > 0 ? u : -u);
    t = (b > 0 ? v : -v);
    return g;
}

i128 abs128(i128 x) {
    return x < 0 ? -x : x;
}

i128 gcd128(i128 a, i128 b) {
    a = abs128(a);
    b = abs128(b);
    while (b != 0) {
        i128 r = a % b;
        a = b;
        b = r;
    }
    return a;
}

void print128(i128 x) {
    if (x == 0) {
        cout << "0\n";
        return;
    }
    string s;
    while (x > 0) {
        s.push_back(char('0' + x % 10));
        x /= 10;
    }
    reverse(s.begin(), s.end());
    cout << s << '\n';
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<int64> x(n), y(n);
    for (int i = 0; i < n; ++i) cin >> x[i] >> y[i];

    vector<int64> dx, dy;
    for (int i = 1; i < n; ++i) {
        dx.push_back(x[i] - x[0]);
        dy.push_back(y[i] - y[0]);
    }

    // Construct a lattice vector (bx, gy), where gy is gcd of all dy values.
    int64 gy = 0;
    i128 bx = 0;

    for (int i = 0; i < (int)dx.size(); ++i) {
        int64 s, t;
        int64 new_gy = extended_gcd_signed(gy, dy[i], s, t);
        bx = (i128)s * bx + (i128)t * dx[i];
        gy = new_gy;
    }

    // If every difference has y = 0, rank is at most one.
    if (gy == 0) {
        cout << -1 << '\n';
        return 0;
    }

    // Remove the y component from every generator; remaining vectors are horizontal.
    i128 horizontal_gcd = 0;
    for (int i = 0; i < (int)dx.size(); ++i) {
        i128 horizontal = (i128)dx[i] - (i128)(dy[i] / gy) * bx;
        horizontal_gcd = gcd128(horizontal_gcd, horizontal);
    }

    // No nonzero horizontal generator means rank is still below two.
    if (horizontal_gcd == 0) {
        cout << -1 << '\n';
        return 0;
    }

    print128(horizontal_gcd * gy);
    return 0;
}
