***頂点クエリのみに対応しています***  
また、非常に使いにくい形式になっていると思われます  
# HLD
木に対するパスクエリ、部分木クエリなどを解きます。（セグ木とかを使えるもののみ）  
<br>
## 下準備  
・node_typeをいじる  
<br>
## コンストラクタ
```cpp
//デフォルトで、
hld<typename S, auto op /*以下省略可能*/ , auto e = []{return S();}, typename F = int, auto mapping = []{}, auto composition = []{}, auto id = []{}> AAA(int _n, int _r = random)
```
**・ 概要 :** S, op, eはACLのsegtreeのようなもの  
_nは頂点数  
_rは根（部分木クエリを使うときは指定する）
<br>
## 構築
```cpp
void add_edge(int u, int v)
```
add_edgeで無向グラフを追加し、  
```cpp
void build(vector<S> v)
```
その後、引数に「頂点に書かれている値」を渡し、buildする  
<br>
## 機能
```cpp
//セグ木を用いた場合、Θ(log^2 N)
S prod(int l, int r);
//同様で、Θ(log N)
S prod(int r);
S get(int p);
void set(int p, S x);
//Θ(log N)
int lca(int l, int r);
int dist(int l, int r);

//- 以下あれば -
//セグ木で、Θ(log^2 N)
void apply(int l, int r, F x);
//同様で、Θ(log N)
void apply(int r, F x);
void add(int p, S x);
```

<br>
<br>

verified?  
https://judge.yosupo.jp/submission/281120  
https://judge.yosupo.jp/submission/281117
