# graph
静的な有向グラフを、辺のリストから高速に構築します。  
構築後のグラフは、各頂点に隣接する頂点のリストを`for`文などで走査できます。  
**無向グラフを扱う場合は、各辺について双方向に辺を追加してください。**
<br>
## コンストラクタ
```cpp
graph<T> G(int n, const auto &E);
graph<T> G;
```
* `T`は辺の情報を格納する型です．辺の行き先のみの場合は`int`，重み付きなら`pair<int, cost_type>`などを指定します．
<br>
* `graph(n, E)`は，頂点数`n`，辺のリスト`E`からグラフを構築します．
* * 辺のリスト`E`の各要素は，`{u, v}`のように始点`u`と辺の情報`v`（型は`T`）で構成される必要があります．
* `graph()`は，空のグラフを作成します．後から`build`で構築できます．
### 制約
* `n >= 0`
* `E`の各要素`{u, v}`について，`0 <= u < n`
### 計算量
* `graph(n, E)`: `O(n + |E|)`
* `graph()`: `O(1)`
<br>
## build
```cpp
void build(int n, const auto &E);
```
頂点数`n`，辺のリスト`E`からグラフを構築します．既にグラフが構築されている場合でも，この関数で再構築できます．
### 制約
* `n >= 0`
* `E`の各要素`{u, v}`について，`0 <= u < n`
### 計算量
* `O(n + |E|)`
<br>
## operator[]
```cpp
auto operator[](int p);
```
頂点`p`から出る辺のリストを取得します．返されるオブジェクトは範囲for文で走査できます．
### 制約
* `0 <= p < n`
### 計算量
* O(1)
<br>
## 使用例
```cpp
#include <bits/stdc++.h>
using namespace std;

//ここにソースコードを貼り付ける

int main() {
    // 頂点数 4
    int n = 4;

    // 重み付き有向グラフ
    // E: {始点, {終点, 重み}} のリスト
    vector<pair<int, pair<int, int>>> E = {
        {0, {1, 10}},
        {0, {2, 20}},
        {1, {2, 30}},
        {1, {3, 40}},
        {2, {3, 50}}
    };

    // グラフ構築
    // 辺の情報(行き先, 重み)の型は pair<int, int>
    graph<pair<int, int>> G(n, E);

    // 各頂点から出る辺を走査
    for (int i = 0; i < n; i++) {
        cout << "Vertex " << i << ":";
        for (auto [to, cost] : G[i]) {
            cout << " (" << to << ", " << cost << ")";
        }
        cout << endl;
    }

    return 0;
}

/*
出力:
Vertex 0: (1, 10) (2, 20)
Vertex 1: (2, 30) (3, 40)
Vertex 2: (3, 50)
Vertex 3:
*/
```