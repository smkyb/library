# coordinate_compression
与えられた複数の数値の配列の値を、それらの相対的な順序を保ったまま、重複なく小さい非負整数に圧縮する機能を提供します。

## 関数
```cpp
template<typename T, typename... Args>
std::vector<T> compression(std::vector<T>& v, Args&... args)
```

### 引数

*   `v`: 圧縮したい数値の配列（`std::vector<T>`）。この配列は、関数内で直接変更されます。
*   `args...`: 圧縮したい追加の数値の配列（`std::vector<T>`）。これらも関数内で直接変更されます。

### 戻り値

*   圧縮後のユニークな値のソートされた配列（`std::vector<T>`）。

### 制約

*   入力される配列の要素型 `T` は、比較可能である必要があります（`operator<` および `operator==` が定義されていること）。
*   引数として渡される配列は、すべて同じ要素型 `T` である必要があります。

### 計算量

*   $O(N \log N)$
    *   ここで $N$ は、すべての入力配列の要素数の合計です。
    *   内部でソートと重複除去が行われるため、この計算量となります。

### 使用例

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

// compression 関数が定義されているヘッダーファイルなどをインクルード

int main() {
    std::vector<int> v1 = {10, 20, 10, 30};
    std::vector<int> v2 = {5, 20, 15};

    // v1 と v2 の値を圧縮
    std::vector<int> unique_values = compression(v1, v2);

    // 圧縮後の v1 の内容
    // v1 は {0, 1, 0, 2} のようになる
    std::cout << "Compressed v1: ";
    for (int val : v1) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    // 圧縮後の v2 の内容
    // v2 は {3, 1, 4} のようになる (unique_values のインデックスに対応)
    std::cout << "Compressed v2: ";
    for (int val : v2) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    // 圧縮されたユニークな値
    // unique_values は {5, 10, 15, 20, 30} のようになる
    std::cout << "Unique values: ";
    for (int val : unique_values) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    return 0;
}
```