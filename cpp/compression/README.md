# Coordinate Compression

与えられた複数の `std::vector` に含まれる値の範囲を圧縮（座圧）します。
これにより、元の値の大小関係を保ったまま、より小さい値の集合で表現することができます。

## 関数
```cpp
template<typename T, typename... Args>
vector<T> compression(vector<T>& v, Args&... args)
```

・`v`: 圧縮したい `std::vector` です。このベクターの値は、関数内で変更されます。
・`args...`: 圧縮したい他の `std::vector` を可変長引数で指定します。これらのベクターの値も、関数内で変更されます。

### 制約
・圧縮対象の `std::vector` は、同じ型 `T` である必要があります。
・`std::vector` の要素は、比較可能である必要があります (`operator<` が定義されていること)。

### 返り値
圧縮後のユニークな値のソートされた `std::vector` を返します。これは、各元の `std::vector` の要素を、この返り値のベクターにおけるインデックスに置き換えた結果となります。

### 計算量
・`N`: 全ての `std::vector` を合わせた要素数の総和とします。
・`M`: 全ての `std::vector` に含まれるユニークな値の総数とします。

・マージ処理: O(N)
・ソート処理: O(M log M)
・ユニーク化処理: O(M)
・適用処理: O(N log M)

総合して、**O(N + M log M)** です。

## 使用例

```cpp
#include <iostream>
#include <vector>
#include <algorithm> // for sort, unique, lower_bound

// compression 関数の定義 (ソースコードから抜粋)
void _compression_merge(...){}
template<typename T, typename... Args>
void _compression_merge(vector<T>& C, vector<T>& v, Args&... args){
    for(auto& i : v) C.push_back(i);
    _compression_merge(C, args...);
}
void _compression_apply(...){}
template<typename T, typename... Args>
void _compression_apply(vector<T>& C, vector<T>& v, Args&... args){
    for(auto& i : v) i = lower_bound(C.begin(), C.end(), i) - C.begin();
    _compression_apply(C, args...);
}
template<typename T, typename... Args>
vector<T> compression(vector<T>& v, Args&... args){
    vector<T> C;
    _compression_merge(C, v, args...);
    sort(C.begin(), C.end());
    C.erase(unique(C.begin(), C.end()), C.end());
    _compression_apply(C, v, args...);
    return C;
}

int main() {
    vector<int> a = {5, 2, 8, 2, 5, 1};
    vector<int> b = {10, 8, 3, 10};

    vector<int> unique_sorted_values = compression(a, b);

    // 圧縮後の a
    // a は {2, 1, 3, 1, 2, 0} に変更される
    for (int val : a) {
        std::cout << val << " ";
    }
    std::cout << std::endl; // 出力: 2 1 3 1 2 0

    // 圧縮後の b
    // b は {4, 3, 0, 4} に変更される
    for (int val : b) {
        std::cout << val << " ";
    }
    std::cout << std::endl; // 出力: 4 3 0 4

    // 圧縮後のユニークな値
    // unique_sorted_values は {1, 2, 3, 5, 8, 10} になる
    for (int val : unique_sorted_values) {
        std::cout << val << " ";
    }
    std::cout << std::endl; // 出力: 1 2 3 5 8 10

    return 0;
}
```