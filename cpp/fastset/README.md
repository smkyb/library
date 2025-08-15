# fastset
要素の集合を管理するデータ構造です。
要素は非負整数です。
<br>
## コンストラクタ
```cpp
fastset(int _n)
```
* 要素の最大値を`_n`として，空の集合を生成します。
### 制約
* `_n`は正の値であること。
### 計算量
* O(_n)
<br>
## insert
```cpp
void insert(int x)
```
集合に要素`x`を追加します。
### 制約
* `0 <= x < _n`
### 計算量
* O(1)
<br>
## erase
```cpp
void erase(int x)
```
集合から要素`x`を削除します。
### 制約
* `0 <= x < _n`
### 計算量
* O(1)
<br>
## count
```cpp
bool count(int x) const
```
集合に要素`x`が含まれているかを返します。
### 制約
* `0 <= x < _n`
### 計算量
* O(1)
<br>
## lower_bound
```cpp
int lower_bound(int x)
```
集合内で、`x`以上の最小の要素を返します。
存在しない場合は -1 を返します。
### 制約
* `0 <= x < _n`
### 計算量
* O(1)
<br>
## less_bound
```cpp
int less_bound(int x)
```
集合内で、`x`未満の最大の要素を返します。
存在しない場合は -1 を返します。
### 制約
* `0 <= x < _n`
### 計算量
* O(1)
<br>