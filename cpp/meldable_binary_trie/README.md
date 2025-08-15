# meldable_binary_trie
キーと値のペアを格納し、効率的な操作を提供するデータ構造です。キーは非負整数で、値は可換モノイドを形成します。

<br>

## コンストラクタ
```cpp
template<typename T, typename S, auto op, auto e>
meldable_binary_trie(T p, const S &x);

template<typename T, typename S, auto op, auto e>
meldable_binary_trie(node_t *x);
```
* `T`: キーの型 (unsigned integer)
* `S`: 値の型 (可換モノイド)
* `op`: 値に対するモノイド演算
* `e`: 値の単位元

* `meldable_binary_trie(T p, const S &x)`: キー `p`、値 `x` を持つ要素で初期化されたトライを構築します。
* `meldable_binary_trie(node_t *x)`: 指定されたノードをルートとするトライを構築します。

### 制約
* `T`は符号なし整数型である必要があります。
* `op`は結合律を満たす二項演算である必要があります。
* `e`は`op`の単位元である必要があります。

### 計算量
* O(1) (あるいは、内部ノードの構築にかかる時間)

<br>

## insert
```cpp
void insert(T v, const S &x)
```
キー `v`、値 `x` の要素をトライに挿入します。キー `v` が既に存在する場合、その値は `op(既存の値, x)` で更新されます。

### 制約
* `0 <= v`

### 計算量
* O(log(max(T)))

<br>

## count
```cpp
int count(T v) const
```
キー `v` を持つ要素の数を返します。

### 制約
* `0 <= v`

### 計算量
* O(log(max(T)))

<br>

## size
```cpp
int size() const
```
トライに含まれる要素の総数を返します。

### 制約
* なし

### 計算量
* O(1)

<br>

## meld
```cpp
static node_t *meld(meldable_binary_trie &l, meldable_binary_trie &r);
static node_t *meld(node_t *l, node_t *r);
static node_t *meld(node_t *l, node_t *r, int bit);
```
2つのトライ `l` と `r` をマージし、新しいトライのルートノードへのポインタを返します。マージされたトライは、`l` と `r` のすべての要素を含みます。

### 制約
* `l` および `r` は有効なトライのルートノードである必要があります。

### 計算量
* O(log(max(T)))

<br>

## split
```cpp
pair<meldable_binary_trie, meldable_binary_trie> split(int p);
static pair<node_t*, node_t*> split(node_t *l, int p);
```
トライを、指定された要素数 `p` で2つのトライに分割します。最初のトライには最初の `p` 個の要素が、2番目のトライには残りの要素が含まれます。

### 制約
* `0 <= p <= size()`

### 計算量
* O(log(max(T)))

<br>

## split_one
```cpp
meldable_binary_trie split_one();
node_t *split_one(node_t *l);
node_t *split_one_rev(node_t *l);
```
トライから要素を1つ取り出し、それを新しいトライとして返します。元のトライからはその要素が削除されます。

### 制約
* トライが空でないこと。

### 計算量
* O(log(max(T)))

<br>

## all_prod
```cpp
S all_prod() const
```
トライに含まれるすべての要素の値に対して、モノイド演算 `op` を適用した結果を返します。

### 制約
* なし

### 計算量
* O(1)

<br>

## init
```cpp
void init(T p, const S &x)
```
トライをキー `p`、値 `x` の要素で初期化します。既存のトライの内容は破棄されます。

### 制約
* `0 <= p`

### 計算量
* O(1)

<br>

## get_size
```cpp
static int get_size(node_t *ptr);
```
指定されたノードをルートとする部分木のノード数を計算します。

### 制約
* `ptr` は有効なノードまたは `node_t::nil` である必要があります。

### 計算量
* O(N)、ここで N は部分木のノード数