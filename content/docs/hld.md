## 例

$N$ 頂点の木が与えられ、頂点 $i$ には最初整数 $a_i$ が書かれています。以下の $Q$ 個のクエリを処理してください。

- `0 u x`: 頂点 $u$ に書かれた整数に $x$ を加算する。
- `1 u v`: 頂点 $u$ と頂点 $v$ を結ぶ単純パス上の頂点に書かれた整数の総和を出力する。

```cpp
#include <bits/stdc++.h>
#include <atcoder/segtree>
using namespace std;

// ここに hld.cpp を貼り付け

using S = long long;
S op(S a, S b) { return a + b; }
S e() { return 0; }

int main() {
    int n, q;
    cin >> n >> q;
    vector<S> a(n);
    for (int i = 0; i < n; ++i) {
        cin >> a[i];
    }

    // 総和は可換なので commutative = true を指定できる
    hld_v<atcoder::segtree<S, op, e>, true> tree(n);
    for (int i = 0; i < n - 1; ++i) {
        int u, v;
        cin >> u >> v;
        tree.add_edge(u, v);
    }
    tree.build(a);

    while (q--) {
        int type;
        cin >> type;
        if (type == 0) {
            int u;
            S x;
            cin >> u >> x;
            tree.set(u, tree.get(u) + x);
        } else {
            int u, v;
            cin >> u >> v;
            cout << tree.prod(u, v) << '\n';
        }
    }
}
```

辺に値を持たせる問題の場合は、`hld_e<atcoder::segtree<S, op, e>, true> tree(n);` として `tree.add_edge(u, v, w);` と `tree.build();` に置き換えるだけで同じように扱えます。

## 関数一覧

| 関数 | 対象 | 説明 |
| --- | --- | --- |
| `hld_v<T, comm>(n, root = 0)` / `hld_e<T, comm>(n, root = 0)` | 共通 | 頂点数 `n`、根 `root` で初期化する |
| `add_edge(u, v)` | `hld_v` | 辺 `(u, v)` を追加する |
| `add_edge(u, v, x)` | `hld_e` | 重み（値）`x` の辺 `(u, v)` を追加する |
| `build(v)` / `build()` | `hld_v` / `hld_e` | 木を構築する（`hld_v` は初期頂点配列 `v` を渡す） |
| `prod(u, v)` | 共通 | `u`–`v` パス上の積を取得する（非可換時は $u \to v$ 順） |
| `subtree_prod(v)` | 共通 | `v` を根とする部分木の積を取得する |
| `get(v)` / `get(u, v)` | `hld_v` / `hld_e` | 頂点 `v` または辺 `(u, v)` の現在の値を取得する |
| `set(v, x)` / `set(u, v, x)` | `hld_v` / `hld_e` | 頂点 `v` または辺 `(u, v)` の値を `x` に更新する |
| `add(v, x)` / `add(u, v, x)` | `hld_v` / `hld_e` | 頂点 `v` または辺 `(u, v)` の値に `op(現在値, x)` を反映する |
| `apply(u, v, x)` / `subtree_apply(v, x)` | 共通（遅延セグ木時） | パスまたは部分木全体に作用素 `x` を適用する |
| `lca(u, v)` | 共通 | 2 頂点 `u, v` の最小共通祖先を返す |
| `dist(u, v)` | 共通 | 2 頂点 `u, v` 間の**辺数**を返す |

## 関数の説明

### コンストラクタ

```cpp
(1) hld_v<T<S, op, e>, commutative> tree(int n, int root = 0)
(2) hld_v<T<S, op, e, F, mapping, composition, id>, commutative> tree(int n, int root = 0)
(3) hld_e<T<S, op, e>, commutative> tree(int n, int root = 0)
(4) hld_e<T<S, op, e, F, mapping, composition, id>, commutative> tree(int n, int root = 0)
```

- (1), (2): 頂点に値を持つ、頂点数 `n`・根 `root` の木を作成します。
- (3), (4): 辺に値を持つ、頂点数 `n`・根 `root` の木を作成します。
- (2), (4) のように遅延セグメント木（`atcoder::lazy_segtree` など）を指定した場合は、`apply` / `subtree_apply` も利用できます。
- `commutative` は省略可能（デフォルト `false`）で、演算 `op` が可換な場合は `true` を指定できます。

**制約**

- $0 \le n$、$n > 0$ のとき $0 \le \text{root} < n$
- `T<...>` はテンプレート引数が `<S, op, e>` または `<S, op, e, F, mapping, composition, id>` であり、空区間クエリで単位元 `e()` を返すデータ構造であること
- C++20 以降

**計算量**

- $O(n)$

### add_edge

```cpp
(1) void tree.add_edge(int u, int v)       // hld_v
(2) void tree.add_edge(int u, int v, S x)  // hld_e
```

無向辺 `(u, v)` を追加します。`hld_e` では辺の初期値 `x` も指定します。

**制約**

- $0 \le u, v < n$、$u \neq v$
- `build` 前にちょうど $n - 1$ 回呼び、全体が連結な木になること

**計算量**

- $O(1)$

### build

```cpp
(1) void tree.build(vector<S> v)  // hld_v
(2) void tree.build()             // hld_e
```

追加した辺から木を構築します。`hld_v` では各頂点の初期値配列 `v` を渡します。

**制約**

- `add_edge` をちょうど $n - 1$ 回呼んだ後に 1 回だけ呼ぶこと
- `hld_v` では `v.size() >= n`

**計算量**

- $O(n)$

### prod

```cpp
S tree.prod(int u, int v)
```

頂点 `u` から `v` への単純パス上の積を返します。`hld_v` は両端 `u, v` を含む頂点、`hld_e` はパス上の辺を対象とし、`u == v` のとき `hld_e` は `e()` を返します。

**制約**

- $0 \le u, v < n$

**計算量**

- $O(\log^2 n)$（`T` の区間取得が $O(\log n)$ の場合）

### subtree_prod

```cpp
S tree.subtree_prod(int v)
```

頂点 `v` を根とする部分木内の積を返します（`hld_v` は `v` を含む部分木内の全頂点、`hld_e` は部分木内の全辺）。

**制約**

- $0 \le v < n$

**計算量**

- $O(\log n)$（`T` の区間取得が $O(\log n)$ の場合）

### get

```cpp
(1) S tree.get(int v)         // hld_v
(2) S tree.get(int u, int v)  // hld_e
```

頂点 `v` または辺 `(u, v)` の現在の値を返します。

**制約**

- $0 \le u, v < n$
- `hld_e` では木上に辺 `(u, v)` が存在すること（存在チェックなし）

**計算量**

- $O(1)$（`segtree` 使用時）

### set

```cpp
(1) void tree.set(int v, S x)         // hld_v
(2) void tree.set(int u, int v, S x)  // hld_e
```

頂点 `v` または辺 `(u, v)` の値を `x` に更新します。

**制約**

- $0 \le u, v < n$
- `hld_e` では木上に辺 `(u, v)` が存在すること（存在チェックなし）

**計算量**

- $O(\log n)$（`segtree` 使用時）

### add

```cpp
(1) void tree.add(int v, S x)         // hld_v
(2) void tree.add(int u, int v, S x)  // hld_e
```

頂点 `v` または辺 `(u, v)` の値を `op(現在値, x)` に更新します。

**制約**

- $0 \le u, v < n$
- `hld_e` では木上に辺 `(u, v)` が存在すること（存在チェックなし）
- `T` が `add` を持つ場合のみ利用可能（`atcoder::segtree` / `atcoder::lazy_segtree` 使用時は `set(v, get(v) + x)` を使用）

**計算量**

- $O(\log n)$（`segtree` 使用時）

### apply

```cpp
void tree.apply(int u, int v, F x)
```

遅延セグメント木を `T` に指定した場合に利用でき、`u`–`v` パス上の全要素に作用素 `x` を一括適用します。

**制約**

- $0 \le u, v < n$
- `commutative = false` では作用素 `F` が反転不変であること

**計算量**

- $O(\log^2 n)$

### subtree_apply

```cpp
void tree.subtree_apply(int v, F x)
```

遅延セグメント木を `T` に指定した場合に利用でき、頂点 `v` の部分木内の全要素に作用素 `x` を一括適用します。

**制約**

- $0 \le v < n$
- `commutative = false` では作用素 `F` が反転不変であること

**計算量**

- $O(\log n)$

### lca

```cpp
int tree.lca(int u, int v)
```

2 頂点 `u, v` の最小共通祖先（LCA）を返します。

**制約**

- $0 \le u, v < n$

**計算量**

- $O(\log n)$

### dist

```cpp
int tree.dist(int u, int v)
```

2 頂点 `u, v` 間の最短パスに含まれる**辺の本数**を返します（`hld_e` でも重みの総和ではありません）。

**制約**

- $0 \le u, v < n$

**計算量**

- $O(\log n)$
