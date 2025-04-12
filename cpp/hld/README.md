# HLD
木に対するパスクエリ、部分木クエリなどを解きます。（セグ木とかを使えるもののみ）  
hld/hld_treev : 頂点  
hld/hld_treee : 辺  
hld/hld_treen : jumpをする  
<br>
## 下準備
・上部の、「非可換か」「add使えるか」「区間作用あるか」のチェックリストを埋める  
・template引数をいじる  
・node_typeをいじる  
<br>
## コンストラクタ
```cpp
hld_tree(int _n, vector<int>& g, const vector<int>& start, const vector<S>& v, int r = 0)
```
**・ 概要 :** _nはサイズ、vは頂点に書かれている値、rは根。  
gとstartに関しては、同じフォルダに入っているcsr.cppで読み込んだものを使う。（有向無向どちらでも）  
<br>
## 機能
```cpp
//セグ木を用いた場合、Θ(log^2 N)
S prod(int l, int r);
//同様に、Θ(log N)
S prod(int r);
S get(int p);
void set(int p, S x);
//Θ(log N)
int lca(int l, int r);
int dist(int l, int r);

//セグ木で、Θ(log^2 N)
void apply(int l, int r, F x);
//同様に、Θ(log N)
void apply(int r, F x);
void add(int p, S x);
```
<br>

# csr
HLDで使うグラフを標準入力から読み込むもの。
有向と無向がある。
```cpp
void read_csr_ud_graph(vector<int>& g, vector<int>& start, int m, int num = -1);
void read_csr_d_graph(vector<int>& g, vector<int>& start, int m, int num = -1);
```
mは辺数、numは添え字調節用（入力が1idxだったら-1、0idxだったら0）
