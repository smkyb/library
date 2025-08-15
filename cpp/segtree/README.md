# segtree
セグメント木を実装したデータ構造です。区間に対するクエリを効率的に行うことができます。

## コンストラクタ

```cpp
segtree(unsigned n)
segtree(const vector<S>& v)
```

*   `S`: 要素の型。
*   `op`: 結合法則を満たす二項演算子（例: `std::plus<>`, `std::bit_and<>`）。
*   `e`: `op`の単位元（例: `std::plus<>`なら0, `std::bit_and<>`なら-1）。
*   `n`: セグメント木を構築する際の要素数。
*   `v`: 初期化に用いる要素のリスト。

**例:**

```cpp
// 合計を計算するセグメント木
segtree<int, std::plus<>, std::[]{return 0;}> seg_sum(10);

// 初期値を持つセグメント木
vector<int> initial_values = {1, 2, 3, 4, 5};
segtree<int, std::plus<>, std::[]{return 0;}> seg_init(initial_values);
```

## メソッド

### `get(int pos)`

指定された位置 `pos` の要素を取得します。

*   `pos`: 要素のインデックス (0-indexed)。
*   **計算量:** O(1)

### `set(int pos, const S& x)`

指定された位置 `pos` の要素を `x` に変更します。

*   `pos`: 要素のインデックス (0-indexed)。
*   `x`: 新しい要素の値。
*   **計算量:** O(log n)

### `add(int pos, const S& x)`

指定された位置 `pos` の要素に `x` を加算（`op` に基づいた更新）します。

*   `pos`: 要素のインデックス (0-indexed)。
*   `x`: 加算する値。
*   **制約:** `node_type` が `add` 関数を持っている必要があります。
*   **計算量:** `op` の計算量に依存します。

### `prod(int left, int right)`

指定された区間 `[left, right)` の要素に対する `op` の累積結果を返します。

*   `left`: 区間の開始インデックス (0-indexed、inclusive)。
*   `right`: 区間の終了インデックス (0-indexed、exclusive)。
*   **計算量:** O(log n)

### `all_prod()`

セグメント木全体（全ての要素）に対する `op` の累積結果を返します。

*   **計算量:** O(1)