# dijkstra
単一始点最短経路問題を解きます。
<br>
## コンストラクタ
```cpp
dijkstra()
```
デフォルトコンストラクタです。
<br>
## operator()
```cpp
pair<vector<long long>, vector<int>> operator()(int n, const vector<pair<int, pair<int, long long>>> &E, const vector<int> &s)
```
* `n`: 頂点数（0-indexed）
* `E`: 辺のリスト。`E[i] = {u, {v, cost}}` は頂点 `u` から頂点 `v` へコスト `cost` で向かう辺を表します。
* `s`: 始点のリスト。
<br>
返り値は、始点からの最短距離のリストと、最短経路の復元に用いる親頂点のリストのペアです。
### 制約
* `n` は正の整数
* `E` の各要素 `u`, `v` は `0 <= u, v < n` を満たす
* `s` の各要素は `0 <= s_i < n` を満たす
* 辺のコストは非負
### 計算量
* O(E log V) （Eは辺の数、Vは頂点数）
<br>
## operator()
```cpp
template<typename graphlike>
pair<vector<long long>, vector<int>> operator()(int n, graphlike &G, const vector<int> &s)
```
グラフ構造 `G` を直接受け取るオーバーロードです。`G` は `operator[](int)` で頂点 `p` に接続する辺のリスト（`pair<int, long long>`のコンテナ）を返す必要があります。
* `n`: 頂点数（0-indexed）
* `G`: グラフ構造体。`G[p]` は頂点 `p` から出る辺のリスト（`pair<int, long long>`のコンテナ）を返します。
* `s`: 始点のリスト。
<br>
返り値は、始点からの最短距離のリストと、最短経路の復元に用いる親頂点のリストのペアです。
### 制約
* `n` は正の整数
* `s` の各要素は `0 <= s_i < n` を満たす
* 辺のコストは非負
### 計算量
* O(E log V) （Eは辺の数、Vは頂点数）