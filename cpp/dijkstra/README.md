# Dijkstra

グラフ上の最短経路問題を解くためのアルゴリズムです。

<br>

## 構造体 `graph`

グラフの隣接リスト表現を管理します。

```cpp
struct graph {
    vector<pair<int, long long>> G;
    vector<int> idx;
    struct ref_t {
        vector<pair<int, long long>>::iterator begin_, end_;
        auto begin() const noexcept { return begin_; }
        auto end() const noexcept { return end_; }
        auto size() const noexcept { return end_ - begin_; }
        pair<int, long long> operator[](int p) const { return begin_[p]; }
    };
    graph() = default;
    graph(int n, const vector<pair<int, pair<int, long long>>> &E) { build(n, E); }
    void build(int n, const vector<pair<int, pair<int, long long>>> &E) {
        G.resize(E.size());
        idx.resize(n + 1);
        for (auto &[u, v] : E) {
            idx[u + 1]++;
        }
        for (int i = 1; i <= n; i++) idx[i] += idx[i - 1];
        auto C = idx;
        for (auto &[u, v] : E) {
            G[C[u]++] = v;
        }
    }
    ref_t operator[](int p) { return {G.begin() + idx[p], G.begin() + idx[p + 1]}; }
};
```

*   `graph(int n, const vector<pair<int, pair<int, long long>>> &E)`: 頂点数`n`と辺のリスト`E`からグラフを構築します。辺のリスト`E`は `{u, {v, cost}}` の形式です。

### 制約

*   `n`は正の整数。
*   `E`の各要素 `{u, {v, cost}}` において、`0 <= u < n`、`0 <= v < n`、`cost >= 0`。

### 計算量

*   コンストラクタ: O(V + E) (V: 頂点数, E: 辺数)

<br>

## Dijkstra クラス

ダイクストラ法による最短経路計算を行います。

```cpp
struct _Dijkstra {
    // ... graph struct ...
    struct _comp { bool operator()(const pair<int, long long> &l, const pair<int, long long> &r) { return l.second > r.second; } };
    
    // 一般呼び出し用
    pair<vector<long long>, vector<int>> operator()(int n, const vector<pair<int, pair<int, long long>>> &E, const vector<int> &s);
    
    // template<typename graphlike, ...> operator() ... (内部利用)
};
```

### `operator()`

ダイクストラ法を実行し、各頂点への最短距離と最短経路の直前の頂点を返します。

```cpp
pair<vector<long long>, vector<int>> operator()(int n, const vector<pair<int, pair<int, long long>>> &E, const vector<int> &s)
```

*   `n`: グラフの頂点数。
*   `E`: グラフの辺のリスト。各要素は `{u, {v, cost}}` の形式です。
    *   `u`: 始点
    *   `v`: 終点
    *   `cost`: 辺の重み (非負)
*   `s`: 始点となる頂点のリスト。

### 制約

*   `n`は正の整数。
*   `E`の各要素 `{u, {v, cost}}` において、`0 <= u < n`、`0 <= v < n`、`cost >= 0`。
*   `s`の各要素 `i` において、`0 <= i < n`。

### 計算量

*   O(E log V)

### 返り値

*   `vector<long long>`: 各頂点への最短距離。到達不可能な場合は `-1`。
*   `vector<int>`: 各頂点への最短経路における直前の頂点。始点や到達不可能な頂点は `-1`。

<br>