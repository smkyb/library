# Fast I/O

このライブラリは、標準入出力の速度を向上させるためのC++の入出力ストリーム実装です。

## 機能

*   **高速な整数・文字列入力**: `FIstream` クラスは、標準入力から整数や文字列を高速に読み込むためのバッファリングされた入力を提供します。
*   **高速な整数・文字列出力**: `FOstream` クラスは、標準出力へ整数、文字、文字列、浮動小数点数を高速に書き込むためのバッファリングされた出力を提供します。
*   **operator overloading**: `cin` と `cout` のエイリアスが提供されており、標準の C++ ストリームライクなインターフェースで使用できます。

## 使用例

```cpp
#include <iostream>
#include <string>
#include <vector>

// FIstream, FOstream, cin, cout, istream, ostream の定義が含まれていると仮定

int main() {
    // 整数入力
    int a;
    cin >> a;

    // 文字列入力
    std::string s;
    cin >> s;

    // 整数出力
    cout << a << std::endl;

    // 文字列出力
    cout << "Hello, " << s << "!" << std::endl;

    // 浮動小数点数出力
    double pi = 3.1415926535;
    cout << pi << std::endl;

    // 複数入力
    int x, y;
    cin >> x >> y;
    cout << x + y << std::endl;

    return 0;
}
```

## クラス詳細

### `FIstream`

*   **`_getchar()`**: バッファから1文字読み込みます。バッファが空の場合は標準入力から読み込みます。
*   **`ignore_space()`**: 空白文字（ASCIIコード 0x20 以下）をスキップし、最初の非空白文字を返します。
*   **`_read(T& res)`**: テンプレート関数。整数型 `T` の値を読み込みます。空白文字、符号（`-`）を処理し、数値をパースします。
*   **`operator>>(T& x)`**: 整数型 `T` の値を `x` に読み込みます。
*   **`operator>>(char& x)`**: 最初の非空白文字を `x` に読み込みます。
*   **`operator>>(string& x)`**: 空白文字をスキップした後、次の空白文字が出現するまで文字を読み込み、`std::string` `x` に格納します。

### `FOstream`

*   **`_write()`**: バッファの内容を標準出力に書き込みます。
*   **`_putchar(char c)`**: バッファに1文字書き込みます。バッファが満杯の場合は、書き込む前にバッファの内容をフラッシュします。
*   **`_write_i(T x)`**: テンプレート関数。整数型 `T` の値を高速に文字列に変換してバッファに書き込みます。負の数も処理します。
*   **`operator<<(const T& x)`**: 整数型 `T` の値を標準出力に書き込みます。
*   **`operator<<(char x)`**: 文字 `x` を標準出力に書き込みます。
*   **`operator<<(const char* x)`**: Cスタイルの文字列 `x` を標準出力に書き込みます。
*   **`operator<<(char* x)`**: `const char*` のバージョンにキャストして呼び出します。
*   **`operator<<(double x)`**: `double` 型の値をフォーマットして出力します。`snprintf` を使用して、最大15桁の精度で浮動小数点数を出力します。NaN も処理します。
*   **`operator<<(long double x)`**: `long double` 型の値をフォーマットして出力します。`snprintf` を使用して、最大15桁の精度で浮動小数点数を出力します。NaN も処理します。
*   **`operator<<(const string& x)`**: `std::string` `x` を標準出力に書き込みます。
*   **`~FOstream()`**: デストラクタ。バッファに残っている内容があれば、標準出力に書き込みます。

## マクロ

*   `cin`: `_cin` ( `FIstream` のインスタンス) のエイリアス。
*   `istream`: `FIstream` のエイリアス。
*   `cout`: `_cout` ( `FOstream` のインスタンス) のエイリアス。
*   `ostream`: `FOstream` のエイリアス。