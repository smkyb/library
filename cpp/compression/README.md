# compression
1つ以上の配列（`vector`）をまとめて座標圧縮します．  
引数として与えられた配列の要素は，圧縮後の値（0-indexedの整数）に直接書き換えられます．
<br>
## compression
```cpp
template<typename T, typename... Args>
vector<T> compression(vector<T>& v, Args&... args);
```
* 引数として与えられた1つ以上の `vector<T>` をまとめて座標圧縮します．
* 返り値は，圧縮に使われたユニークな値のリスト（昇順）です．このリスト `C` を用いて，圧縮後の値 `i` から元の値 `C[i]` を復元できます．
### 制約
* `T` はソート可能である必要があります．
### 計算量
入力される全要素数をNとすると，
* O(N log N)
<br>
## 使用例
```cpp
#include <bits/stdc++.h>
using namespace std;

//ここにソースコードを貼り付ける

int main() {
    vector<int> a = {10, 50, 30, 100, 30};
    vector<int> b = {30, 20, 50};

    cout << "Before compression:" << endl;
    cout << "a: ";
    for (int i = 0; i < a.size(); ++i) cout << a[i] << (i == a.size() - 1 ? "" : " ");
    cout << endl;
    cout << "b: ";
    for (int i = 0; i < b.size(); ++i) cout << b[i] << (i == b.size() - 1 ? "" : " ");
    cout << endl << endl;
    
    // aとbをまとめて座標圧縮
    auto C = compression(a, b);

    cout << "After compression:" << endl;
    cout << "a: ";
    for (int i = 0; i < a.size(); ++i) cout << a[i] << (i == a.size() - 1 ? "" : " ");
    cout << endl;
    cout << "b: ";
    for (int i = 0; i < b.size(); ++i) cout << b[i] << (i == b.size() - 1 ? "" : " ");
    cout << endl << endl;
    
    cout << "Unique values list (C):" << endl;
    for (int i = 0; i < C.size(); ++i) cout << C[i] << (i == C.size() - 1 ? "" : " ");
    cout << endl;

    // 出力:
    // Before compression:
    // a: 10 50 30 100 30
    // b: 30 20 50
    // 
    // After compression:
    // a: 0 3 2 4 2
    // b: 2 1 3
    // 
    // Unique values list (C):
    // 10 20 30 50 100
}
```