# graph
有向グラフの隣接リストを効率的に構築し、各頂点からの出辺を高速に参照できるデータ構造です。

<br>
## コンストラクタ
```cpp
graph(int n, const auto &E);
```
*   `n`: グラフの頂点数。`0`から`n-1`までの頂点が存在すると仮定します。
*   `E`: 辺のリスト。`vector<pair<int, int>>`や`vector<array<int, 2>>`のような、`{u, v}`の形式で辺を表すコレクションを想定しています。
### 制約
*   `n`は正の整数であり、かつ大きすぎないこと。
*   `E`に含まれるすべての頂点`u`, `v`について、`0 <= u < n` および `0 <= v < n` が成り立つこと。
*   **多重辺に対応していますが、自己ループも処理されます**。
### 計算量
*   O(n + |E|) (ここで`|E|`は辺の数)

<br>
## build
```cpp
void build(int n, const auto &E);
```
デフォルトコンストラクタで`graph`オブジェクトを生成した後、このメソッドでグラフを構築します。引数の意味はコンストラクタと同様です。
### 制約
*   コンストラクタと同様。
*   このメソッドは一度だけ呼び出すべきです。複数回呼び出した場合、既存のグラフデータが上書きされます。
### 計算量
*   O(n + |E|)

<br>
## operator[]
```cpp
ref_t operator[](int p);
```
頂点`p`に隣接する頂点（つまり、頂点`p`から出る辺の終点）のコレクションを`ref_t`型で返します。
`ref_t`はイテレータ、サイズ取得、インデックスアクセスをサポートする軽量なオブジェクトです。

*   `auto begin() const noexcept`: 隣接頂点の範囲の先頭イテレータを返します。
*   `auto end() const noexcept`: 隣接頂点の範囲の末尾イテレータを返します。
*   `auto size() const noexcept`: 頂点`p`の出次数を返します。
*   `T operator[](int i) const`: `i`番目の隣接頂点を返します。
### 制約
*   `0 <= p < n` (構築時に指定した頂点数`n`の範囲内)
### 計算量
*   O(1)

<br>
## 使用例
```cpp
#include <vector>
#include <iostream>
#include <utility> // for std::pair
#include <array>   // for std::array (optional, if E uses array)

// (ここに graph struct をコピー)
template<typename T>
struct graph {
    std::vector<T> G;
    std::vector<int> idx;
    struct ref_t {
        typename std::vector<T>::iterator begin_, end_;
        auto begin() const noexcept {return begin_;}
        auto end() const noexcept {return end_;}
        auto size() const noexcept {return end_ - begin_;}
        T operator[](int p) const {return begin_[p];}
    };
    
    graph() = default;
    graph(int n, const auto &E){
        build(n, E);
    }
    
    void build(int n, const auto &E){
        G.resize(E.size());
        idx.resize(n+1);
        for(auto &[u, v] : E){
            idx[u+1]++;
        }
        for(int i = 1; i <= n; i++) idx[i] += idx[i-1];
        auto C = idx;
        for(auto &[u, v] : E){
            G[C[u]++] = v;
        }
    }
    
    ref_t operator[](int p) {return {G.begin()+idx[p], G.begin()+idx[p+1]};}
};


int main() {
    int n = 4; // 頂点数
    std::vector<std::pair<int, int>> edges = {
        {0, 1},
        {0, 2},
        {1, 2},
        {2, 0},
        {2, 3},
        {3, 3}
    };

    // コンストラクタでグラフを構築
    graph<int> g(n, edges);

    std::cout << "Graph Edges:" << std::endl;
    for (int i = 0; i < n; ++i) {
        std::cout << "Node " << i << " neighbors (out-edges): ";
        // 範囲for文で隣接頂点をイテレート
        for (int neighbor : g[i]) {
            std::cout << neighbor << " ";
        }
        std::cout << "(count: " << g[i].size() << ")" << std::endl;
    }

    // 特定の隣接頂点へのアクセス
    if (g[0].size() > 0) {
        std::cout << "First neighbor of node 0: " << g[0][0] << std::endl;
    }

    return 0;
}
```
出力例：
```
Graph Edges:
Node 0 neighbors (out-edges): 1 2 (count: 2)
Node 1 neighbors (out-edges): 2 (count: 1)
Node 2 neighbors (out-edges): 0 3 (count: 2)
Node 3 neighbors (out-edges): 3 (count: 1)
First neighbor of node 0: 1
```