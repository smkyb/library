# dijkstra
ダイクストラ法を用いて、単一（または複数）始点からの最短経路を計算します。  
**辺の重みは非負である必要があります．**
<br>
## dijkstra
```cpp
pair<vector<long long>, vector<int>> dijkstra(int n, const vector<pair<int, pair<int, long long>>> &E, const vector<int> &s);
```
頂点数 `n`、辺のリスト `E`、始点のリスト `s` を持つ有向グラフの最短経路を計算します。
* `E` は `vector<pair<int, pair<int, long long>>>` 型で、各要素は `{u, {v, cost}}` （頂点 `u` から `v` へのコスト `cost` の辺）の形式です。
* 返り値は `pair<vector<long long>, vector<int>>` です。
* * `first` は各頂点への最短距離のリストです。到達不可能な頂点の距離は `-1` となります。
* * `second` は最短路木における各頂点の親のリストです。始点や到達不可能な頂点の親は `-1` となります。
### 制約
* `0 <= 頂点番号 < n`
* 辺の重みは非負
### 計算量
頂点数をN、辺数をMとすると，
* O(M log N)
<br>
## 使用例
```cpp
#include <bits/stdc++.h>
using namespace std;

//ここにソースコードを貼り付ける

int main() {
    // 頂点数 6, 辺数 9
    int n = 6;
    vector<pair<int, pair<int, long long>>> E = {
        {0, {1, 7}}, {0, {2, 9}}, {0, {5, 14}},
        {1, {2, 10}}, {1, {3, 15}},
        {2, {3, 11}}, {2, {5, 2}},
        {3, {4, 6}},
        {5, {4, 9}}
    };
    
    // 始点 0
    vector<int> s = {0};

    auto [dist, path] = dijkstra(n, E, s);

    cout << "Distances from source 0:" << endl;
    for (int i = 0; i < n; ++i) {
        cout << "Vertex " << i << ": ";
        if (dist[i] == -1) {
            cout << "unreachable" << endl;
        } else {
            cout << dist[i] << endl;
        }
    }
    cout << endl;

    cout << "Predecessors in shortest path tree:" << endl;
    for (int i = 0; i < n; ++i) {
        cout << "Vertex " << i << ": " << path[i] << endl;
    }

    // 出力:
    // Distances from source 0:
    // Vertex 0: 0
    // Vertex 1: 7
    // Vertex 2: 9
    // Vertex 3: 20
    // Vertex 4: 20
    // Vertex 5: 11
    //
    // Predecessors in shortest path tree:
    // Vertex 0: -1
    // Vertex 1: 0
    // Vertex 2: 0
    // Vertex 3: 2
    // Vertex 4: 5
    // Vertex 5: 2
}
```