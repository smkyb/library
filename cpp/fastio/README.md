# Faster IO Library

このライブラリは、標準入出力 (`cin`, `cout`) を高速化するためのものです。

## 機能

*   **高速な整数読み込み:** `operator>>` を使用して `int` などの整数型を高速に読み込みます。
*   **高速な文字読み込み:** `operator>>` を使用して `char` 型を高速に読み込みます。空白文字をスキップします。
*   **高速な文字列読み込み:** `operator>>` を使用して `std::string` を高速に読み込みます。空白文字をスキップし、空白文字が現れるまで読み込みます。
*   **高速な整数書き込み:** `operator<<` を使用して `int` などの整数型を高速に標準出力へ書き込みます。
*   **高速な文字書き込み:** `operator<<` を使用して `char` 型を高速に標準出力へ書き込みます。
*   **高速な文字列書き込み:** `operator<<` を使用して `const char*` や `std::string` を高速に標準出力へ書き込みます。
*   **高速な浮動小数点数書き込み:** `operator<<` を使用して `double` や `long double` を高速に標準出力へ書き込みます。

## 使用方法

このライブラリを使用するには、以下の `#define` をソースコードの先頭に記述してください。

```cpp
#include <iostream> // または <cstdio>
#include <string>
#include <cstring> // for memcpy
#include <cassert> // for assert
#include <cmath> // for isnan

// Faster IO Libraryの定義をここにコピー＆ペースト

#define cin _cin
#define istream FIstream
#define cout _cout
#define ostream FOstream
```

その後、通常の `std::cin` や `std::cout` と同様に使用できます。

```cpp
#include <iostream>
#include <string>
#include <vector>

// Faster IO Libraryの定義をここにコピー＆ペースト

#define cin _cin
#define istream FIstream
#define cout _cout
#define ostream FOstream

int main() {
    int n;
    cin >> n; // 高速に整数を読み込む

    std::string s;
    cin >> s; // 高速に文字列を読み込む

    std::vector<int> arr(n);
    for (int i = 0; i < n; ++i) {
        cin >> arr[i]; // 配列の各要素を高速に読み込む
    }

    cout << "Read " << n << " elements." << endl; // 高速に文字列と整数を出力
    cout << "String: " << s << endl;

    for (int x : arr) {
        cout << x << " "; // 配列の要素を高速に出力
    }
    cout << endl;

    double d = 3.14159;
    cout << "Double: " << d << endl; // 高速に浮動小数点数を出力

    return 0;
}
```