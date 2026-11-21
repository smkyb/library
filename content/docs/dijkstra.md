## 概要

辺の重みが非負である有向グラフに対して、始点集合からの**最短距離**と**最短経路の直前頂点**を求めるアルゴリズムです。

隣接リストをそのまま渡す形式と、辺リストを渡す形式の両方に対応しています。無向グラフで使いたい場合は、双方向の有向辺を追加して渡してください。

## 関数一覧

| 関数 | 説明 |
| --- | --- |
| `dijkstra(n, graph, starts)` | 隣接リスト `graph` と始点配列 `starts` から `{dist, prev}` を求める |
| `dijkstra(n, edges, starts)` | 辺リスト `edges` と始点配列 `starts` から `{dist, prev}` を求める |

## 各関数の説明

### dijkstra

```cpp
(1) pair<vector<long long>, vector<int>> dijkstra(int n, Graph graph, vector<int> starts)
(2) pair<vector<long long>, vector<int>> dijkstra(int n, vector<pair<int, pair<int, long long>>> edges, vector<int> starts)
```

始点集合 `starts` から各頂点への最短距離 `dist` と、最短経路上の直前頂点 `prev` の組 `{dist, prev}` を返します。

- (1): 隣接リスト `graph`（`graph[u]` が `{次頂点, 重み}` を列挙できるコンテナ、例: `vector<vector<pair<int, long long>>>`）を渡します。
- (2): `{始点 u, {終点 v, 重み cost}}` を並べた辺配列 `edges` を渡します。
- 未到達頂点 `v` は `dist[v] == -1`、始点および未到達頂点は `prev[v] == -1` となります。

**制約**

- 頂点番号は 0-indexed（$0 \le u, v < n$）
- 辺の重みは非負の `long long`
- 経路長の合計が `LLONG_MAX` を超える経路は緩和されない

**計算量**

- 時間 $O((V + E) \log V)$
- 空間 $O(V + E)$

## 具体例

### 問題

$N$ 頂点 $M$ 辺の重み付き有向グラフが与えられます。$i$ 番目の辺は頂点 $u_i$ から頂点 $v_i$ へ向かい、長さは $w_i \ge 0$ です。

頂点 $0$ から頂点 $N - 1$ への最短距離と、それを達成する経路の頂点列を 1 つ出力してください。到達できない場合は `-1` を出力してください。

### 解答例

```cpp
#include <bits/stdc++.h>
using namespace std;

// ここに dijkstra.cpp を貼り付け

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;
    vector<vector<pair<int, long long>>> G(n);
    for (int i = 0; i < m; ++i) {
        int u, v;
        long long w;
        cin >> u >> v >> w;
        G[u].push_back({v, w});
    }

    auto [dist, prev] = dijkstra(n, G, {0});

    if (dist[n - 1] == -1) {
        cout << -1 << '\n';
        return 0;
    }

    vector<int> path;
    for (int v = n - 1; v != -1; v = prev[v]) {
        path.push_back(v);
    }
    reverse(path.begin(), path.end());

    cout << dist[n - 1] << '\n';
    for (int i = 0; i < (int)path.size(); ++i) {
        cout << path[i] << (i + 1 == (int)path.size() ? '\n' : ' ');
    }
}
```
