# Fast IO Library

このライブラリは、標準入出力の高速化を目的としたC++のクラスを提供します。

## クラス

### `FIstream` (Input Stream)

標準入力からの読み込みを高速化します。

#### コンストラクタ

```cpp
FIstream()
```
デフォルトコンストラクタです。

#### 演算子オーバーロード

*   **`operator>>(T& x)`**:
    指定された型の変数`x`に、標準入力から値を読み込みます。整数型（`int`, `long long`など）および`char`、`string`に対応しています。

    ```cpp
    // 整数を読み込む例
    int a;
    cin >> a;

    // 文字を読み込む例
    char c;
    cin >> c;

    // 文字列を読み込む例
    std::string s;
    cin >> s;
    ```

    *   **制約**:
        *   読み込む値は、対応する型として有効な形式である必要があります。
        *   `char`や`string`の場合、空白文字（スペース、タブ、改行など）はスキップされます。

    *   **計算量**:
        *   `int`, `long long`などの整数型: O(log10(値))
        *   `char`: O(1)
        *   `string`: O(文字列長)

### `FOstream` (Output Stream)

標準出力への書き込みを高速化します。

#### コンストラクタ

```cpp
FOstream()
```
デフォルトコンストラクタです。

#### 演算子オーバーロード

*   **`operator<<(const T& x)`**:
    指定された型の値`x`を標準出力に書き込みます。整数型（`int`, `long long`など）、`char`、`const char*`、`double`、`long double`、`string`に対応しています。

    ```cpp
    // 整数を出力する例
    cout << 123;

    // 文字を出力する例
    cout << 'A';

    // 文字列リテラルを出力する例
    cout << "Hello";

    // std::stringを出力する例
    std::string message = "World";
    cout << message;

    // doubleを出力する例
    double pi = 3.14159;
    cout << pi;
    ```

    *   **制約**:
        *   `double`および`long double`は、`snprintf`を用いて最大15桁の精度で出力されます。
        *   `double`または`long double`がNaN（非数）の場合、「nan」と出力されます。

    *   **計算量**:
        *   整数型: O(log10(値))
        *   `char`: O(1)
        *   `const char*`: O(文字列長)
        *   `std::string`: O(文字列長)
        *   `double`, `long double`: O(1) （内部バッファリングにより、実質的な計算量は定数に近い）

#### デストラクタ

```cpp
~FOstream()
```
クラスのインスタンスが破棄される際に、バッファに残っているデータを標準出力にフラッシュします。

## マクロ

*   `cin`: `FIstream`クラスのグローバルインスタンス`_cin`へのエイリアスです。
*   `istream`: `FIstream`クラスへのエイリアスです。
*   `cout`: `FOstream`クラスのグローバルインスタンス`_cout`へのエイリアスです。
*   `ostream`: `FOstream`クラスへのエイリアスです。

これらのマクロを使用することで、C++標準ライブラリの`std::cin`や`std::cout`と同様の感覚で、高速な入出力を行うことができます。

```cpp
#include <iostream>
#include <string>
#include <vector>

// Fast IO Library をインクルードする代わりに、直接ソースコードをコピー＆ペーストして使用します。
// または、必要に応じてヘッダーファイルとして保存しインクルードしてください。

/*
struct FIstream { ... };
struct FOstream_Pre { ... };
struct FOstream { ... };
FIstream _cin;
FOstream _cout;
#define cin _cin
#define istream FIstream
#define cout _cout
#define ostream FOstream
*/

int main() {
    std::ios_base::sync_with_stdio(false); // 標準ライブラリの同期を解除 (fastioライブラリ自体で対応済みですが、併用も可能です)
    // cin.tie(NULL); // cinとcoutのtieを解除 (fastioライブラリ自体で対応済みですが、併用も可能です)

    int n;
    cin >> n; // FIstream を使用

    std::vector<int> nums(n);
    for (int i = 0; i < n; ++i) {
        cin >> nums[i]; // FIstream を使用
    }

    for (int i = 0; i < n; ++i) {
        cout << nums[i] << " "; // FOstream を使用
    }
    cout << "\n"; // FOstream を使用

    std::string word;
    cin >> word; // FIstream を使用
    cout << "Read: " << word << "\n"; // FOstream を使用

    double val = 3.1415926535;
    cout << "Pi: " << val << "\n"; // FOstream を使用

    return 0;
}
```