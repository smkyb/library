# meldable_binary_trie
符号なし整数をキーとした要素の集合を管理し、マージ、分割、順位に基づく区間のモノイド積の取得を行えるデータ構造です。
<br>
## コンストラクタ
```cpp
/*1*/ meldable_binary_trie<T, S, op, e> trie();
/*2*/ meldable_binary_trie<T, S, op, e> trie(T p, const S &x);
```
* キーの型（符号なし整数）`T`
* ACLと同様にモノイドの型`S`, 演算`op`, 単位元`e`
を定義する必要があります。
<br>
* * 1 : 空のデータ構造を作ります。
* * 2 : キー `p` 、値 `x` を持つ要素が1つだけ含まれるデータ構造を作ります。
### 制約
* `T` は符号なし整数型（`unsigned int`, `unsigned long long` など）
### 計算量
* O(1)
<br>
## size
```cpp
int size() const
```
データ構造に含まれる要素の数を返します。
### 計算量
* O(1)
<br>
## meld
```cpp
static node_t* meld(meldable_binary_trie &l, meldable_binary_trie &r)
```
2つのデータ構造 `l` と `r` をマージします。
※ `l` と `r` は破壊されます。
返り値は内部ノードのポインタであり、以下のようにして新しいデータ構造を作ることができます。
`meldable_binary_trie trie(meldable_binary_trie::meld(l, r));`
### 計算量
`T`の最大値を`m`とすると、
* 償却O(log(m))
<br>
## split
```cpp
pair<meldable_binary_trie, meldable_binary_trie> split(int p)
```
キーの昇順に見て、先頭から `p` 個の要素を持つデータ構造と、残りの要素を持つデータ構造のペアに分割して返します。
※ 元のデータ構造は破壊されます。
### 制約
* `0 <= p <= size()`
### 計算量
`T`の最大値を`m`とすると、
* O(log(m))
<br>
## split_one
```cpp
meldable_binary_trie split_one()
```
データ構造から1つの要素を分離し、その要素のみを持つデータ構造を返します。
※ 元のデータ構造は1要素分小さく（破壊）されます。
### 制約
* `size() > 0`
### 計算量
`T`の最大値を`m`とすると、
* O(log(m))
<br>
## prod_l
```cpp
S prod_l(int n)
```
キーの昇順に見て、左から `n` 個の要素の値の総積を返します。
### 制約
* `0 <= n <= size()`
### 計算量
`T`の最大値を`m`とすると、
* O(log(m))
<br>
## prod_r
```cpp
S prod_r(int n)
```
キーの昇順に見て、右から `n` 個の要素の値の総積を返します。
### 制約
* `0 <= n <= size()`
### 計算量
`T`の最大値を`m`とすると、
* O(log(m))
<br>
## prod_lr
```cpp
S prod_lr(int l, int r)
```
キーの昇順に見て、`l` 番目から `r-1` 番目までの要素の値の総積を返します。
### 制約
* `0 <= l <= r <= size()`
### 計算量
`T`の最大値を`m`とすると、
* O(log(m))
<br>
## all_prod
```cpp
S all_prod() const
```
含まれるすべての要素の値の総積を返します。
### 計算量
* O(1)
<br>
## init
```cpp
void init(T p, const S &x)
```
データ構造を破棄し、キー `p` 、値 `x` を持つ要素が1つだけ含まれる状態に初期化します。
### 計算量
* O(1)
<br>
## 使用例
```cpp
#include <bits/stdc++.h>
using namespace std;

//ここにソースコードを貼り付ける

using S = long long;
S op(S a, S b) { return a + b; }
S e() { return 0; }
using Trie = meldable_binary_trie<unsigned int, S, op, e>;

int main() {
    // 1つの要素からなるTrie木をそれぞれ作成
    Trie t1(5, 10); // キー5, 値10
    Trie t2(3, 20); // キー3, 値20
    Trie t3(8, 30); // キー8, 値30

    // t1 と t2 をマージして t12 を作成 (t1, t2 は破壊される)
    Trie t12(Trie::meld(t1, t2));
    
    // さらに t3 とマージして t を作成
    Trie t(Trie::meld(t12, t3));

    // 要素の数は 3
    cout << "size: " << t.size() << endl;

    // キーの昇順 (3, 5, 8) なので、それぞれの値は 20, 10, 30
    // 全体の和は 60
    cout << "all_prod: " << t.all_prod() << endl;

    // 左から2つの要素 (キー3, 5) の和は 20 + 10 = 30
    cout << "prod_l(2): " << t.prod_l(2) << endl;

    // 左から1番目以上3番目未満の要素 (キー5, 8) の和は 10 + 30 = 40
    cout << "prod_lr(1, 3): " << t.prod_lr(1, 3) << endl;

    // 先頭から2つの要素と、残りの1つに分割
    auto [left, right] = t.split(2);
    
    cout << "left size: " << left.size() << ", sum: " << left.all_prod() << endl;
    cout << "right size: " << right.size() << ", sum: " << right.all_prod() << endl;

    // 出力:
    // size: 3
    // all_prod: 60
    // prod_l(2): 30
    // prod_lr(1, 3): 40
    // left size: 2, sum: 30
    // right size: 1, sum: 30

    return 0;
}
```