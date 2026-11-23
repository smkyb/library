## 例

長さ $N$ の列があり、$i$ 番目の要素は互いに相異なる非負整数キー $p_i$ と一次関数 $f_i(x) = a_i x + b_i$ を持ちます。以下の $Q$ 個のクエリを順に処理してください。

- `0 i p a b`: $i$ 番目の要素のキーを $p$、関数を $a x + b$ に更新する（更新後も列内のキーは互いに相異なる）。
- `1 l r x`: $f_{r-1}(f_{r-2}(\dots f_l(x) \dots))$ を $998244353$ で割った余りを出力する。
- `2 l r`: 区間 $[l, r)$ の要素をキーの昇順にソートする。
- `3 l r`: 区間 $[l, r)$ の要素をキーの降順にソートする。

```cpp
#include <bits/stdc++.h>
#include <atcoder/modint>
using namespace std;

// ここに sortable_segtree.cpp を貼り付け

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
    vector<pair<uint32_t, F>> init(n);
    for (int i = 0; i < n; ++i) {
        uint32_t p;
        long long a, b;
        cin >> p >> a >> b;
        init[i] = {p, {a, b}};
    }

    sortable_segtree<uint32_t, F, op, e> seg(init);

    while (q--) {
        int type;
        cin >> type;
        if (type == 0) {
            int i;
            uint32_t p;
            long long a, b;
            cin >> i >> p >> a >> b;
            seg.set(i, p, {a, b});
        } else if (type == 1) {
            int l, r;
            long long x;
            cin >> l >> r >> x;
            F res = seg.prod(l, r);
            cout << (res.a * x + res.b).val() << '\n';
        } else if (type == 2) {
            int l, r;
            cin >> l >> r;
            seg.sort(l, r);
        } else {
            int l, r;
            cin >> l >> r;
            seg.sort_desc(l, r);
        }
    }
}
```

## 関数一覧

| 関数 | 説明 |
| --- | --- |
| `sortable_segtree<T, S, op, e, comm>(v)` | `(キー, 値)` の配列 `v` で初期化する |
| `set(p, key, val)` | 位置 `p` の要素を `(key, val)` に書き換える |
| `prod(l, r)` | 半開区間 $[l, r)$ の値の総積を取得する |
| `sort(l, r)` | 半開区間 $[l, r)$ をキーの昇順にソートする |
| `sort_desc(l, r)` | 半開区間 $[l, r)$ をキーの降順にソートする |

## 関数の説明

### コンストラクタ

```cpp
sortable_segtree<T, S, op, e, commutative> seg(vector<pair<T, S>> v)
```

長さ `n = v.size()` の列を `(キー, 値)` のペア配列 `v` で初期化して作ります。

- キーの型 `T`（`uint32_t` または `uint64_t`）
- モノイドの型 `S`
- 二項演算を計算する関数 `S op(S a, S b)`
- 単位元を返す関数 `S e()`
- `commutative` は省略可能（デフォルト `false`）で、演算 `op` が可換な場合は `true` を指定するとより高速・省メモリになります。

**制約**

- $0 \le n \le 524287$
- `T` は `uint32_t` または `uint64_t`
- 全要素のキーが互いに相異なること（重複キーは非対応）
- `op` は結合則 $\text{op}(\text{op}(a, b), c) = \text{op}(a, \text{op}(b, c))$ を満たし、`e()` は単位元であること
- C++20 以降、64bit GCC / Clang 環境（コピー不可・ムーブのみ）

**計算量**

- $O(n)$

### set

```cpp
void seg.set(int p, T key, S val)
```

位置 `p` の要素をキー `key`・値 `val` に書き換えます。

**制約**

- $0 \le p < n$
- 更新後も列全体のキーが互いに相異なること

**計算量**

- $O(\log n + \log K)$（$K$ はキーの値域上限）

### prod

```cpp
S seg.prod(int l, int r)
```

半開区間 $[l, r)$ の値の総積 `op(val[l], ..., val[r - 1])` を返します。`l == r` のときは `e()` を返します（全区間の積は `prod(0, n)` で取得します）。

**制約**

- $0 \le l \le r \le n$

**計算量**

- $O(\log n + \log K)$

### sort

```cpp
void seg.sort(int l, int r)
```

半開区間 $[l, r)$ の要素をキーの**昇順**にソートします。

**制約**

- $0 \le l \le r \le n$

**計算量**

- 償却 $O(\log n + \log K)$

### sort_desc

```cpp
void seg.sort_desc(int l, int r)
```

半開区間 $[l, r)$ の要素をキーの**降順**にソートします。

**制約**

- $0 \le l \le r \le n$

**計算量**

- 償却 $O(\log n + \log K)$
