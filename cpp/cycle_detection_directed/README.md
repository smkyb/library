# cycle_detection_directed
有向グラフからサイクルを1つ検出し，それを構成する辺のインデックスのリストを返します．
<br>
## cycle_detection_directed
```cpp
vector<int> cycle_detection_directed(int n, const auto &e);
```
頂点数 `n` と辺のリスト `e` を持つ有向グラフからサイクルを1つ検出します．
* `e` は `vector<pair<int, int>>` や `vector<array<int, 2>>` などを想定しています．
* 返り値は `vector<int>` です．
* * サイクルが存在する場合，サイクルを構成する辺の入力`e`におけるインデックスのリストを返します．
* * サイクルが存在しない場合，`{-1}` を返します．
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
    // 頂点数 5, 辺数 4
    int n = 5;
    vector<pair<int, int>> e = {
        {0, 1}, // 辺 0
        {1, 2}, // 辺 1
        {2, 1}, // 辺 2
        {3, 4}  // 辺 3
    };

    auto edge_indices = cycle_detection_directed(n, e);

    if (edge_indices.size() == 1 && edge_indices[0] == -1) {
        cout << "No cycle detected." << endl;
    } else {
        cout << "Cycle detected!" << endl;
        cout << "Edges in cycle (indices): ";
        for (int i = 0; i < edge_indices.size(); ++i) {
            cout << edge_indices[i] << (i == edge_indices.size() - 1 ? "" : " ");
        }
        cout << endl;
        // 出力:
        // Cycle detected!
        // Edges in cycle (indices): 1 2
    }

    return 0;
}
```