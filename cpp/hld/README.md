# hld
木に対するパスクエリ，部分木クエリなどを解きます．（パスクエリはモノイド，部分木クエリは可換モノイド）
<br>
## コンストラクタ
```cpp
hld<node_type<S, op, e>> A(int _n, int _r = random);
hld<node_type<S, op, e, F, mapping, composition, id>> A(int _n, int _r = random);
``` 
・`node_type`は，  
・・ - 後述する`apply`を用いる場合，`atcoder::lazy_segtree`相当，  
・・ - 用いない場合，`atcoder::segtree`相当，のデータ構造を入力してください．  
・・そこへACLと同様に`S`, `op`, `e` (, `F`, `mapping`, `composition`, `id`) 
を定義する必要があります．  
<br>
・木の頂点数を`_n`と定めます  
・木の根を`_r`と定め，指定がない場合は無作為に選びます．（部分木クエリを行う場合は必ず指定してください．）
### 制約
・`_n`が正，かつ大きすぎない  
### 計算量
・O(n)  
<br>

以下，`node_type`は関数`set`，`get`，`prod`を持っていると仮定して話を進めます．  
また，`node_type::set`, `node_type::prod`の計算量がO(log(_n))だと仮定します．
<br>
## add_edge
```cpp
void add_edge(int u, int v)
```
木に2頂点`u`, `v`を結ぶ無向辺を追加します．
### 制約
・`0 <= u < _n`  
・`0 <= v < _n`  
・`u != v`
### 計算量
・O(1)
<br>
## build
```cpp
void build(vector<S> v)
```
`add_edge`された辺に対して，頂点の値を`v`として，木を構築します．
### 制約
・`v.size() == _n`  
・コンストラクタの呼び出しから，ちょうど`_n`-1回`add_edge`を呼び出した  
・`add_edge`された辺に対して**構築される木は連結である**
### 計算量
・O(_n)
<br>
## set
```cpp
void set(int p, S x)
```
`p`番目の頂点の値を`x`に変更します．
### 制約
・`0 <= p < _n`
### 計算量
・`node_type::set(p)`と同様
<br>
## add
```cpp
void add(int p, S x)
```
`p`番目の頂点の値に対して`node_type::add`を行います．
### 制約
・`0 <= p < _n`  
・`node_type`が関数add(int, S)を持っている
### 計算量
・`node_type::add(p, x)`と同様
<br>
## get
```cpp
S get(int p)
```
`p`番目の頂点の値を取得します．
### 制約
・`0 <= p < _n`
### 計算量
・`node_type::get(p)`と同様
<br>
## prod
```cpp
S prod(int l, int r)
```
頂点`l`から頂点`r`を結ぶパス(端点を含む)にかかれている値の列を`a0, a1, a2, ..., ai`とした時，`op(op(op(...op(a0, a1), a2), ...), ai)`を返します．
### 制約
・`0 <= l < _n`  
・`0 <= r < _n`
### 計算量
・O(log^2(_n))
<br>
## prod
```cpp
S prod(int r)
```
頂点rを根とする部分木において，各頂点に書かれた値の総積を返します．**（可換性が必要です．）**
### 制約
・`0 <= r < _n`
### 計算量
・O(log(_n))
<br>
## lca
```cpp
int lca(int l, int r)
```
`l`と`r`のLCA（最近共通祖先）を返します．
### 制約
・`0 <= l < _n`  
・`0 <= r < _n`
### 計算量
・O(log(_n))
<br>
## dist
```cpp
int dist(int l, int r)
```
`l`と`r`の距離（端点を含むパスに含まれる辺の数で定義される）を返します．
### 制約
・`0 <= l < _n`  
・`0 <= r < _n`
### 計算量
・O(log(_n))
<br>
## apply
```cpp
void apply(int l, int r, F f)
```
頂点`l`と`r`をつなぐパス（端点を含む）の頂点の値の列を`x`としたとき，  
`x[i] = f(x[i])`をすべての`i`について行います．
### 制約
・`0 <= l < _n`
・`0 <= r < _n`
### 計算量
・O(log^2(_n))
<br>
## apply
```cpp
void apply(int r, F f)
```
頂点rを根とする部分木において，各頂点に書かれた値の列を`x`としたとき，  
`x[i] = f(x[i])`をすべての`i`について行います．
## 制約
・`0 <= r < _n`
### 計算量
・O(log(_n))
