## 例

長さ $N$ の一次関数の列 $f_0, f_1, \dots, f_{N-1}$ があり、最初 $f_i(x) = a_i x + b_i$ です。以下の $Q$ 個のクエリを順に処理してください。

- `0 p c d`: 関数 $f_p(x)$ を $c x + d$ に更新する。
- `1 l r x`: $f_{r-1}(f_{r-2}(\dots f_l(x) \dots))$ を $998244353$ で割った余りを出力する。

```cpp
#include <bits/stdc++.h>
#include <atcoder/modint>
using namespace std;

// ここに segtree.cpp を貼り付け

using mint = atcoder::modint998244353;

struct F {
    mint a, b;
};

// f を適用した後に g を適用する合成関数 g(f(x))
F op(F f, F g) {
    return {g.a * f.a, g.a * f.b + g.b};
}

F e() {
    return {1, 0};
}

int main() {
    int n, q;
    cin >> n >> q;
    vector<F> init_f(n);
    for (int i = 0; i < n; ++i) {
        long long a, b;
        cin >> a >> b;
        init_f[i] = {a, b};
    }

    segtree<F, op, e> seg(init_f);

    while (q--) {
        int type;
        cin >> type;
        if (type == 0) {
            int p;
            long long c, d;
            cin >> p >> c >> d;
            seg.set(p, {c, d});
        } else {
            int l, r;
            long long x;
            cin >> l >> r >> x;
            F res = seg.prod(l, r);
            cout << (res.a * x + res.b).val() << '\n';
        }
    }
}
```

## 関数一覧

| 関数 | 説明 |
| --- | --- |
| `segtree<S, op, e>(n)` | 長さ `n`、全要素 `e()` で初期化して構築する |
| `segtree<S, op, e>(v)` | 配列 `v` の内容で初期化して構築する |
| `init(v)` | 配列 `v` の内容で再初期化する |
| `set(p, x)` | 位置 `p` の要素を `x` に書き換える |
| `add(p, x)` | 位置 `p` の要素を `op(現在値, x)` に更新する |
| `get(p)` / `seg[p]` | 位置 `p` の要素を取得する |
| `prod(l, r)` | 半開区間 $[l, r)$ の総積を取得する |
| `all_prod()` | 全要素の総積を取得する |

## 関数の説明

### コンストラクタ

```cpp
(1) segtree<S, op, e> seg(int n)
(2) segtree<S, op, e> seg(vector<S> v)
```

- モノイドの型 `S`
- 二項演算を計算する関数 `S op(S a, S b)`
- 単位元を返す関数 `S e()`

を定義する必要があります。

- (1): 長さ `n` の数列 `a` を作ります。初期値はすべて `e()` です。
- (2): 長さ `n = v.size()` の数列 `a` を作ります。`v` の内容が初期値となります。

**制約**

- `op` は結合則 $\text{op}(\text{op}(a, b), c) = \text{op}(a, \text{op}(b, c))$ を満たす
- `e()` は単位元 $\text{op}(a, e()) = \text{op}(e(), a) = a$ である
- C++20 以降

**計算量**

- $O(n)$

### init

```cpp
void seg.init(vector<S> v)
```

配列 `v` の内容で長さ `n = v.size()` の数列 `a` を初期化し直します。

**計算量**

- $O(n)$

### set

```cpp
void seg.set(int p, S x)
```

`a[p] = x` に更新します。

**制約**

- $0 \le p < n$

**計算量**

- $O(\log n)$

### add

```cpp
void seg.add(int p, S x)
```

`a[p] = op(a[p], x)` に更新します。

**制約**

- $0 \le p < n$

**計算量**

- $O(\log n)$

### get

```cpp
(1) S seg.get(int p)
(2) S seg[int p]
```

`a[p]` を返します。

**制約**

- $0 \le p < n$

**計算量**

- $O(1)$

### prod

```cpp
S seg.prod(int l, int r)
```

`op(a[l], ..., a[r - 1])` を計算します。`l >= r` のときは `e()` を返します。

**制約**

- $0 \le l, r \le n$（範囲外の端点は配列端にクランプされます）

**計算量**

- $O(\log n)$

### all_prod

```cpp
S seg.all_prod()
```

`op(a[0], ..., a[n - 1])` を計算します。`n == 0` のときは `e()` を返します。

**計算量**

- $O(1)$
