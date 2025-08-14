# sort_segtree
通常のセグメント木に加え，非負整数のキーによる区間ソートを行えるデータ構造です．  
可換性は要求しません．  
<br>
## コンストラクタ
```cpp
/*1*/ sort_segtree<T, S, op, e> seg(int _n);
/*2*/ sort_segtree<T, S, op, e> seg(vector<pair<T, S>> v);
```
・キーの型（非負整数）`T`  
・ACLと同様に`S`, `op`, `e`  
を定義する必要があります．  
<br>
・1 : 初期値`e()`，長さ`_n`の数列`a`を作ります．  
・2 : 初期値が`v`，長さ`v.size()`の数列`a`を作ります．  
### 制約
・`a`の長さが正，かつ大きすぎない  
### 計算量
・O(n log n)
<br>
## set
```cpp
void set(int i, T k, S x)
```
a[i]の，キーを`k`，値を`x`に変更します．
### 制約
・`0 <= i`  
・`0 <= k`  
### 計算量
`T`の最大値を`m`とすると，  
・O(log(m))
<br>
## prod
```cpp
S prod(int l, int r)
```
仕様はACLのセグメント木と同様です．
### 制約
・`0 <= l <= r <= _n`
### 計算量
`T`の最大値を`m`とすると，  
・O(log(m))
<br>
## sort
```cpp
void sort(int l, int r)
```
`a[l], a[l+1], ..., a[r-1]`をキーの昇順でソートします．
### 制約
・`0 <= l <= r <= _n`
### 計算量
`T`の最大値を`m`とすると，  
・償却O(log(m))
<br>
## sort_rev
```cpp
void sort_rev(int l, int r)
```
`a[l], a[l+1], ..., a[r-1]`をキーの降順でソートします．
### 制約
・`0 <= l <= r <= _n`
### 計算量
`T`の最大値を`m`とすると，  
・償却O(log(m))  
