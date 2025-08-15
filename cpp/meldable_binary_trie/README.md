# meldable_binary_trie

このデータ構造は、Trie構造に「meld（結合）」という操作を可能にしたものです。Trieの各ノードは、値、合計値、幅、個数、および4つの子ノードを持ちます。`op`と`e`という汎用的な演算子と単位元を用いることで、様々な応用が可能です。

## テンプレートパラメータ

- `T`: Trieに格納される値の型。unsigned型である必要があります。
- `S`: Trieのノードに格納される合計値の型。
- `op`: 合計値を計算するための二項演算子。
- `e`: `op`の単位元。

## コンストラクタ

```cpp
meldable_binary_trie();
meldable_binary_trie(T p, const S& x);
meldable_binary_trie(node_t* x);
```

- `meldable_binary_trie()`: 空のTrieを構築します。
- `meldable_binary_trie(T p, const S& x)`: 値 `p` と合計値 `x` を持つ単一ノードのTrieを構築します。
- `meldable_binary_trie(node_t* x)`: 与えられたノードをルートとするTrieを構築します。

## メンバー関数

### insert

```cpp
void insert(T v, const S& x);
```

Trieに値 `v` と合計値 `x` を持つ要素を挿入します。

- **制約**:
    - `v` は `T` 型の有効な値であること。
    - `x` は `S` 型の有効な値であること。
- **計算量**: O(log `bit_width`) （`bit_width` は `T` のビット幅）

### count

```cpp
int count(T v) const;
```

Trieに値 `v` が存在するかどうかを返します。存在すれば1、しなければ0を返します。

- **制約**:
    - `v` は `T` 型の有効な値であること。
- **計算量**: O(log `bit_width`)

### size

```cpp
int size() const;
```

Trieに含まれる要素の総数を返します。

- **制約**: なし
- **計算量**: O(1)

### meld

```cpp
static node_t* meld(meldable_binary_trie& l, meldable_binary_trie& r);
static node_t* meld(node_t* l, node_t* r);
```

2つのTrie（またはノード）を結合し、その結果のルートノードへのポインタを返します。結合はTrieの構造を保ちながら行われます。

- **制約**:
    - `l` と `r` は有効なTrieまたはノードであること。
- **計算量**: O(log `bit_width`)

### split

```cpp
pair<meldable_binary_trie, meldable_binary_trie> split(int p);
static pair<node_t*, node_t*> split(node_t* l, int p);
```

Trieを、指定された要素数 `p` で2つのTrieに分割します。`rev` フラグによって分割の順序が変わる場合があります。

- **制約**:
    - `p` は `0 <= p <= size()` の範囲内であること。
- **計算量**: O(log `bit_width`)

### split_one

```cpp
meldable_binary_trie split_one();
node_t* split_one(node_t* l);
node_t* split_one_rev(node_t* l);
```

Trieから要素を1つ取り出し、残りのTrieを返します。どの要素が取り出されるかは、Trieの内部構造や `rev` フラグに依存します。

- **制約**:
    - Trieが空でないこと。
- **計算量**: O(log `bit_width`)

### all_prod

```cpp
S all_prod() const;
```

Trie全体の合計値を返します。

- **制約**: なし
- **計算量**: O(1)

### init

```cpp
void init(T p, const S& x);
```

Trieを初期化し、値 `p` と合計値 `x` を持つ単一ノードのTrieにします。

- **制約**:
    - `p` は `T` 型の有効な値であること。
    - `x` は `S` 型の有効な値であること。
- **計算量**: O(1)

### get_size

```cpp
static int get_size(node_t* ptr);
```

指定されたノード以下のTrieに含まれるノードの総数を計算します。

- **制約**:
    - `ptr` は有効なノードポインタであること。
- **計算量**: O(N) （Nはサブツリー内のノード数）