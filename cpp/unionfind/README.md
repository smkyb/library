# unionfind
要素のグループ分けと、それらの結合・判定を行うデータ構造です。
<br>
## コンストラクタ
```cpp
/*1*/ unionfind(int n)
/*2*/ unionfind(const vector<vector<int>>& v)
```
*   1 : `n`個の要素を持つUnion-Find木を初期化します。最初はどの要素も互いに異なるグループに属しています。
*   2 : `v`を隣接リストとみなし、`v[i]`に含まれる全ての頂点`j`と`i`を結合したUnion-Find木を初期化します。`v.size()`が要素数となります。
### 制約
*   1: `n`が正、かつ大きすぎない
*   2: `v.size()`が正、かつ大きすぎない。`v`の各要素は`0`以上`v.size()-1`未満
### 計算量
*   1: O(n)
*   2: O(N + Mα(N)) （`N`は要素数、`M`は`v`に含まれる辺の総数、`α`はアッカーマン関数の逆関数）
<br>
## root
```cpp
int root(int x)
```
要素`x`が属するグループの代表要素（根）を返します。経路圧縮を行います。
### 制約
*   `0 <= x < _n` （`_n`はコンストラクタで指定した要素数）
### 計算量
*   O(α(_n)) （償却）
<br>
## merge
```cpp
bool merge(int x, int y)
```
要素`x`が属するグループと、要素`y`が属するグループを結合します。
結合が行われた場合`true`を、既に同じグループに属していた場合は`false`を返します。
### 制約
*   `0 <= x < _n`
*   `0 <= y < _n`
### 計算量
*   O(α(_n)) （償却）
<br>
## same
```cpp
bool same(int x, int y)
```
要素`x`と要素`y`が同じグループに属するかどうかを判定します。
同じグループに属している場合`true`を、そうでない場合`false`を返します。
### 制約
*   `0 <= x < _n`
*   `0 <= y < _n`
### 計算量
*   O(α(_n)) （償却）
<br>
## groups
```cpp
vector<vector<int>> groups()
```
現在のUnion-Find木における全てのグループを、各グループに属する要素のリストとして返します。
### 制約
*   なし
### 計算量
*   O(_n)
<br>
## 使用例
```cpp
#include <iostream>
#include <vector>
#include <numeric> // iota用

// unionfind構造体はここに定義されていると仮定

int main() {
    // 5つの要素を持つUnion-Find木を初期化 (0, 1, 2, 3, 4)
    unionfind uf(5);

    std::cout << "Initial groups:" << std::endl;
    for (const auto& group : uf.groups()) {
        std::cout << "  Group: ";
        for (int member : group) {
            std::cout << member << " ";
        }
        std::cout << std::endl;
    }
    // Output:
    //   Group: 0
    //   Group: 1
    //   Group: 2
    //   Group: 3
    //   Group: 4

    // 0と1を結合
    uf.merge(0, 1);
    std::cout << "\nMerged 0 and 1." << std::endl;
    // 1と2を結合
    uf.merge(1, 2);
    std::cout << "Merged 1 and 2." << std::endl;
    // 3と4を結合
    uf.merge(3, 4);
    std::cout << "Merged 3 and 4." << std::endl;

    // 0と2は同じグループか？ -> true
    std::cout << "Are 0 and 2 in the same group? " << (uf.same(0, 2) ? "Yes" : "No") << std::endl;
    // 0と3は同じグループか？ -> false
    std::cout << "Are 0 and 3 in the same group? " " << (uf.same(0, 3) ? "Yes" : "No") << std::endl;

    std::cout << "Groups after merges:" << std::endl;
    for (const auto& group : uf.groups()) {
        std::cout << "  Group: ";
        for (int member : group) {
            std::cout << member << " ";
        }
        std::cout << std::endl;
    }
    // Output:
    //   Group: 0 1 2
    //   Group: 3 4
    
    // グラフの隣接リストからの初期化
    std::vector<std::vector<int>> graph_adj = {
        {1},    // 0 -> 1 と辺がある
        {0, 2}, // 1 -> 0, 2 と辺がある
        {1},    // 2 -> 1 と辺がある
        {4},    // 3 -> 4 と辺がある
        {3}     // 4 -> 3 と辺がある
    };
    unionfind uf_from_graph(graph_adj);
    std::cout << "\nInitialized from graph. Groups:" << std::endl;
    for (const auto& group : uf_from_graph.groups()) {
        std::cout << "  Group: ";
        for (int member : group) {
            std::cout << member << " ";
        }
        std::cout << std::endl;
    }
    // Output:
    //   Group: 0 1 2
    //   Group: 3 4

    return 0;
}
```