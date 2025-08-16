# dijkstra
多点始点に対応したDijkstra法による最短経路アルゴリズムです．
<br>
## 使い方
グローバルに定義された `dijkstra` オブジェクトを関数のように呼び出して使用します．
<br>
```cpp
pair<vector<long long>, vector<int>> dijkstra(int n, const vector<pair<int, pair<int, long long>>> &E, const vector<int> &s)
```
* `n`: グラフの頂点数．頂点IDは `0` から `n-1` です．
* `E`: グラフの辺を表すリスト．
  * 各要素は `{u, {v, cost}}` の形式で，頂点 `u` から頂点 `v` へコスト `cost` の有向辺が存在することを意味します．
  * `cost` は `long long` 型です．
* `s`: 探索の始点となる頂点IDのリスト．複数指定可能です．
<br>
### 戻り値
`pair<vector<long long>, vector<int>>` を返します．
* `first` (`vector<long long>`): 各頂点への最短距離のリスト．
  * `dist[i]` は始点 `s` から頂点 `i` への最短距離です．
  * 到達不可能な頂点への距離は `-1` となります．
* `second` (`vector<int>`): パス復元のための直前ノードのリスト．
  * `path[i]` は最短経路において頂点 `i` に到達する直前の頂点IDです．
  * 始点および到達不可能な頂点の場合は `-1` となります．
<br>
### 制約
* `n` は正の値であること．
* 頂点IDは `0` から `n-1` の範囲内であること．
* 辺のコスト `cost` は非負であること．
* グラフは連結である必要はありません．
### 計算量
* O(`n` log `n` + `|E|`)
  * `n` は頂点数，`|E|` は入力 `E` の要素数（辺の総数）です．