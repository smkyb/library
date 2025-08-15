# meldable_binary_trie

このライブラリは、キーを非負整数、値を任意の型とするデータ構造です。
キーの比較に基づいたマージ（meld）操作、分割（split）操作、挿入（insert）、探索（count）などを効率的に行うことができます。

**注意**:
*   キー（`T`）は非負整数である必要があります。
*   `op`は結合律を満たす演算子（例: `+`, `*`）である必要があります。
*   `e`は`op`の単位元である必要があります。
*   `node_t::nil`は、空のノードを表すための特別なノードです。

## コンストラクタ

### `meldable_binary_trie()`
空のトライを構築します。

### `meldable_binary_trie(T p, const S &x)`
キー`p`、値`x`を持つ単一の要素でトライを構築します。

### `meldable_binary_trie(node_t *x)`
指定されたノード`x`をルートとするトライを構築します。

## メンバ関数

### `void insert(T v, const S &x)`
キー`v`、値`x`を持つ要素をトライに挿入します。
*   **計算量**: O(log(max_key))

### `int count(T v) const`
キー`v`を持つ要素がトライに存在するかどうかを返します。存在すれば1、そうでなければ0を返します。
*   **計算量**: O(log(max_key))

### `int size() const`
トライに含まれる要素の総数を返します。
*   **計算量**: O(1)

### `static node_t *meld(meldable_binary_trie &l, meldable_binary_trie &r)`
2つのトライ`l`と`r`をマージし、マージされたトライのルートノードを返します。元のトライは変更されません。
*   **計算量**: O(log(max_key))

### `pair<meldable_binary_trie, meldable_binary_trie> split(int p)`
トライを2つに分割します。最初のトライは`p`個の要素を持ち、2番目のトライは残りの要素を持ちます。
*   **計算量**: O(log(max_key))

### `meldable_binary_trie split_one()`
トライから要素を1つ取り出し、それ以外の要素で構成される新しいトライを返します。取り出された要素は、返されるトライのルートノードとして扱われます。
*   **計算量**: O(log(max_key))

### `S all_prod() const`
トライに含まれるすべての要素の値に対する`op`演算の結果（総積）を返します。
*   **計算量**: O(1)

### `void init(T p, const S &x)`
トライを初期化します。キー`p`、値`x`を持つ単一の要素でトライを再構築します。
*   **計算量**: O(log(max_key))

### `static int get_size(node_t *ptr)`
指定されたノード`ptr`以下のサブツリーに含まれるノードの総数を計算します。
*   **計算量**: O(N) （Nはサブツリー内のノード数）

## テンプレートパラメータ

*   `typename T`: キーの型。非負整数である必要があります。
*   `typename S`: 値の型。
*   `auto op`: 値の結合に用いる演算子。
*   `auto e`: `op`の単位元。