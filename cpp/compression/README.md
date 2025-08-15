# compression

数値の重複を排除し、元の値に対応する新しい値（インデックス）を生成します。

<br>

## 機能
与えられた複数の `std::vector` の要素をまとめて、重複を排除したソート済みのリストを作成し、元の `std::vector` の各要素をそのリストにおけるインデックスに変換します。

<br>

## 使い方

```cpp
std::vector<int> v1 = {10, 20, 30};
std::vector<int> v2 = {15, 25, 20};
std::vector<int> v3 = {5, 30, 10};

// v1, v2, v3 の要素を圧縮し、元のベクターも更新する
std::vector<int> compressed_values = compression(v1, v2, v3);
```

上記の例では、`v1` は `{0, 1, 2}`、`v2` は `{3, 4, 1}`、`v3` は `{5, 2, 0}` に更新され、`compressed_values` は `{5, 10, 15, 20, 25, 30}` となります。

<br>

## 引数
* `std::vector<T>& v, Args&... args`: 圧縮したい `std::vector` のリスト。型 `T` は比較可能である必要があります。可変長引数テンプレート `Args...` を使用して、任意の数の `std::vector` を渡すことができます。

<br>

## 戻り値
* `std::vector<T>`: 重複が排除され、ソートされた値のリスト。元の `std::vector` の要素は、このリストにおけるそれぞれの値のインデックスに変換されます。

<br>

## 制約
* `std::vector` の要素型 `T` は、`std::sort` および `std::unique` で使用できるように、比較可能である必要があります。
* 渡される `std::vector` の要素は、整数型や浮動小数点数型など、値として比較できる型である必要があります。

<br>

## 計算量
* `N`: 渡された全ての `std::vector` の要素数の合計
* `M`: 重複排除後のユニークな要素数

* `_compression_merge` および `_compression_apply` の内部処理: O(N log M) (ソートと `lower_bound` のため)
* `sort` および `unique`: O(M log M)

全体として、約 **O(N log N)** または **O(N log M)** の計算量となります。