# segtree
セグメント木です。モノイド（`op`, `e`）を扱うことができます。

## コンストラクタ
```cpp
segtree<S, op, e> seg(unsigned n);
segtree<S, op, e> seg(const vector<S>& v);
```
*   `S`: 要素の型
*   `op`: モノイドの演算（例: `std::plus<>`, `std::multiplies<>`）
*   `e`: モノイドの単位元（例: `0`, `1`）
*   `n`: 木のサイズ（構築される配列のサイズ）
*   `v`: 初期値の配列

*   1つ目のコンストラクタは、サイズ`n`で単位元`e()`で初期化されたセグメント木を構築します。
*   2つ目のコンストラクタは、配列`v`で初期化されたセグメント木を構築します。

### 制約
*   `n`は非負整数。
*   `v`の要素数は非負整数。

### 計算量
*   1つ目のコンストラクタ: O(n)
*   2つ目のコンストラクタ: O(n)

## set
```cpp
void set(int pos, const S& x);
```
`pos`番目の要素を`x`に変更します。

### 制約
*   `0 <= pos < siz` (`siz`は構築された配列のサイズ)

### 計算量
*   O(log siz)

## add
```cpp
void add(int pos, const S& x);
```
`pos`番目の要素に`x`を適用します。`op(node[pos], x)`を実行します。

### 制約
*   `0 <= pos < siz`

### 計算量
*   O(log siz)

## get
```cpp
const S& get(int pos) const;
```
`pos`番目の要素を取得します。

### 制約
*   `0 <= pos < siz`

### 計算量
*   O(1)

## prod
```cpp
S prod(int left, int right);
```
`left`番目から`right-1`番目までの要素の範囲積を計算します。

### 制約
*   `0 <= left <= right <= siz`

### 計算量
*   O(log siz)

## all_prod
```cpp
S all_prod();
```
全ての要素の積を計算します。

### 計算量
*   O(1)