# cycle_detection_undirected
無向グラフからサイクルを1つ検出し，それを構成する辺のインデックスのリストを返します．
<br>
## cycle_detection_undirected
```cpp
pair<int, vector<int>> cycle_detection_undirected(int n, const auto &e);
```
頂点数 `n` と辺のリスト `e` を持つ無向グラフからサイクルを1つ検出します．
* `e` は `vector<pair<int, int>>` や `vector<array<int, 2>>` などを想定しています．
* 返り値は `pair<int, vector<int>>` です．
* * サイクルが存在する場合，`first` はサイクル上の始点，`second` はサイクルを構成する辺の入力`e`におけるインデックスのリストです．
* * サイクルが存在しない場合，`{-1, {}}` を返します．
### 制約
* `0 <= 頂点番号 < n`
### 計算量
頂点数をN，辺数をMとすると，
* O(N+M)
<br>
## 使用例
```cpp
#include <bits/stdc++.h>
using namespace std;

//ここにソースコードを貼り付ける

int main() {
    // 頂点数 5, 辺数 5
    int n = 5;
    vector<pair<int, int>> e = {
        {0, 1}, // 辺 0
        {1, 2}, // 辺 1
        {2, 3}, // 辺 2
        {3, 1}, // 辺 3
        {3, 4}  // 辺 4
    };

    auto [start_node, edge_indices] = cycle_detection_undirected(n, e);

    if (start_node != -1) {
        cout << "Cycle detected!" << endl;
        cout << "A vertex on the cycle: " << start_node << endl;
        cout << "Edges in cycle (indices): ";
        for (int i = 0; i < edge_indices.size(); ++i) {
            cout << edge_indices[i] << (i == edge_indices.size() - 1 ? "" : " ");
        }
        cout << endl;
        // 出力:
        // Cycle detected!
        // A vertex on the cycle: 1
        // Edges in cycle (indices): 1 2 3
    } else {
        cout << "No cycle detected." << endl;
    }

    return 0;
}
```