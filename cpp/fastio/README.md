# fast_io
`std::cin`, `std::cout` を置き換えることで、標準入出力を高速化します。  
`#define` により `cin`, `cout` を再定義するため、既存のコードを変更することなく利用できます。
<br>
## 使い方
このソースコードをプログラムの先頭に貼り付け、あとは通常通り `cin`, `cout` を使用してください。  
`>>`, `<<` 演算子で以下の型に対応しています。
* **入力 (`cin`)**: 整数型 (`int`, `long long`など), `char`, `std::string`
* **出力 (`cout`)**: 整数型, `char`, `const char*`, `std::string`, `double`, `long double`
<br>
## 注意事項
* `std::ios_base::sync_with_stdio(false);` や `std::cin.tie(nullptr);` の記述は不要です。
* C言語形式の入出力 (`scanf`, `printf` など) と混ぜて使用しないでください。
* 改行には `std::endl` の代わりに `'\n'` を使用してください。
<br>
## 使用例
```cpp
#include <bits/stdc++.h>
using namespace std;

//ここにソースコードを貼り付ける

int main() {
    // 標準入力から整数 N と、N個の整数を受け取る
    // 入力例:
    // 5
    // 1 2 3 4 5
    
    int n;
    cin >> n;
    
    long long sum = 0;
    for (int i = 0; i < n; ++i) {
        int a;
        cin >> a;
        sum += a;
    }
    
    cout << sum << '\n';
    
    // 出力:
    // 15
    
    return 0;
}
```