# tree_diameter
木の直径（最も遠い2頂点間のパス）の長さと，そのパスを構成する頂点のリストを求めます．
<br>
## tree_diameter
```cpp
pair<long long, vector<int>> tree_diameter(int n, vector<pair<int, pair<int, long long>>> e);
```
頂点数 `n` と辺のリスト `e` を持つ，辺に重みのある木に対し，その直径を求めます．
* `n` は木の頂点数です．
* `e` は `vector<pair<int, pair<int, long long>>>` 型の辺のリストで，`{u, {v, cost}}` は頂点 `u` と `v` を結ぶコスト `cost` の無向辺を表します．
* * `e` のサイズが `n-1` の場合，逆辺は自動で追加されます．
* 返り値は `pair<long long, vector<int>>` です．
* * `first` は直径の長さ（パス上の辺のコストの総和）です．
* * `second` は直径を構成する頂点のリストです．
### 制約
* `1 <= n`
* `0 <= 頂点番号 < n`
* 与えられるグラフは連結な木であること．
* 全ての辺のコストは非負であること．
### 計算量
頂点数をN，辺数をMとすると，
* O(M log N)
<br>
## 使用例
```cpp
#include <bits/stdc++.h>
using namespace std;

//ここにソースコードを貼り付ける

int main() {
    // 頂点数 6
    int n = 6;
    // 辺のリスト {u, {v, cost}}
    vector<pair<int, pair<int, long long>>> e = {
        {0, {1, 3}},
        {1, {2, 2}},
        {1, {3, 5}},
        {3, {4, 1}},
        {3, {5, 4}}
    };

    auto [diameter_length, path] = tree_diameter(n, e);

    cout << "Diameter length: " << diameter_length << endl;
    cout << "Path: ";
    for (int i = 0; i < path.size(); ++i) {
        cout << path[i] << (i == path.size() - 1 ? "" : " -> ");
    }
    cout << endl;

    // 出力:
    // Diameter length: 12
    // Path: 5 -> 3 -> 1 -> 0
}
```