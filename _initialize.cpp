#include <bits/stdc++.h>
using namespace std;
using uint = unsigned;
using ll = long long;
using ull = unsigned long long;
using ld = long double;
#define all(x) begin(x), end(x)
#define elif else if

#include "cpp/fastio/fastio.cpp"

constexpr long long LLINF = (1ll << 62) - 1;
constexpr int INF = (1 << 30) - 1;
constexpr char el = '\n';
template <typename T>
istream& operator>>(istream& ist, vector<T>& v) {
    for (auto& i : v) ist >> i;
    return ist;
}
template <typename T, size_t N>
istream& operator>>(istream& ist, array<T, N>& v) {
    for (auto& i : v) ist >> i;
    return ist;
}
template <typename T>
void read_multi() {}
template <typename T, typename... U>
void read_multi(int n, vector<T>& v, U&&... args) {
    if (n < ssize(v)) {
        cin >> v[n];
        read_multi(args..., n + 1, v);
    }
}
template <typename T, typename... U>
void read_multi(vector<T>& v, U&&... args) { read_multi(args..., 0, v); }
template <typename T, typename U>
ostream& operator<<(ostream& ost, const pair<T, U> p) {
    ost << '{' << p.first << ' ' << p.second << '}';
    return ost;
}
template <typename T>
ostream& operator<<(ostream& ost, const vector<T>& v) {
    for (int i = 0; i < ssize(v); i++) {
        ost << (i ? " " : "") << v[i];
    }
    return ost;
}
template <typename T>
ostream& operator<<(ostream& ost, const vector<vector<T>>& v) {
    for (int i = 0; i < ssize(v); i++) {
        ost << (i ? "\n" : "") << v[i];
    }
    return ost;
}
template <typename T, size_t N>
ostream& operator<<(ostream& ost, const array<T, N>& v) {
    for (int i = 0; i < ssize(v); i++) {
        ost << (i ? " " : "") << v[i];
    }
    return ost;
}
template <typename T, size_t N, size_t M>
ostream& operator<<(ostream& ost, const array<array<T, N>, M>& v) {
    for (int i = 0; i < ssize(v); i++) {
        ost << (i ? "\n" : "") << v[i];
    }
    return ost;
}
template <typename T, typename U>
inline bool chmin(T& a, U b) {
    if (a > b) {
        a = b;
        return true;
    }
    return false;
}
template <typename T, typename U>
inline bool chmax(T& a, U b) {
    if (a < b) {
        a = b;
        return true;
    }
    return false;
}
long long power(long long val, long long num, long long mod) {
    assert(mod >= 0);
    assert(num >= 0);
    long long res = 1;
    val %= mod;
    while (num) {
        if (num & 1) res = (res * val) % mod;
        val = (val * val) % mod;
        num >>= 1;
    }
    return res;
}

#include <atcoder/modint>
using namespace atcoder;
using mint9 = modint998244353;
using mint1 = modint1000000007;
ostream& operator<<(ostream& ost, const mint1& x) {
    ost << x.val();
    return ost;
}
ostream& operator<<(ostream& ost, const mint9& x) {
    ost << x.val();
    return ost;
}
