# unionfind (Disjoint Set Union)
互いに素な集合を管理するデータ構造です．要素の併合や，同じ集合に属するかの判定などを高速に行います．

## コンストラクタ
```cpp
unionfind uf(int n);
unionfind uf(const vector<vector<int>>& v);
```
* `unionfind uf(int n)`:  
  `n`個の要素を持つUnion-Find木を構築します．最初は各要素がそれぞれ異なる集合に属しています．  
* `unionfind uf(const vector<vector<int>>& v)`:  
  `v.size()`個の要素を持つUnion-Find木を構築します．  
  `v[i]`に含まれるすべての要素`j`について，`i`と`j`を併合します．  
### 制約
* `n`は正の整数であること．
* `v`の各要素`j`について `0 <= j < v.size()`であること．
### 計算量
* `unionfind(int n)`: O(n)
* `unionfind(const vector<vector<int>>& v)`: O(N + Mα(N)) (N: 要素数, M: 併合回数)

## root
```cpp
int root(int x);
```
要素`x`が属する集合の代表元（根）を返します．
### 制約
* `0 <= x < n` (nはコンストラクタで指定した要素数)
### 計算量
* ほぼO(α(n)) (償却定数時間)

## merge
```cpp
bool merge(int x, int y);
```
要素`x`と要素`y`が属する集合を併合します．  
既に同じ集合に属していた場合は`false`を，異なる集合だった場合は`true`を返します．
### 制約
* `0 <= x < n`
* `0 <= y < n`
### 計算量
* ほぼO(α(n)) (償却定数時間)

## same
```cpp
bool same(int x, int y);
```
要素`x`と要素`y`が同じ集合に属するかどうかを判定します．  
同じ集合に属する場合は`true`を，異なる集合に属する場合は`false`を返します．
### 制約
* `0 <= x < n`
* `0 <= y < n`
### 計算量
* ほぼO(α(n)) (償却定数時間)

## groups
```cpp
vector<vector<int>> groups();
```
現在のUnion-Find木におけるすべての集合を，要素のリストのリストとして返します．  
例: `{{0, 2}, {1}, {3, 4}}`
### 制約
* なし
### 計算量
* O(n)