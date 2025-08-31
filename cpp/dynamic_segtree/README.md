# dynamic_segtree
区間に対するクエリを解くことができます．  
モノイドの操作`op`と単位元`e`を定義する必要があります．
<br>
## コンストラクタ
```cpp
template <typename T, typename S, auto op, auto e>
dynamic_segtree<T, S, op, e> seg;
```
・キーの型`T`（非負整数である必要があります）  
・値の型`S`  
・モノイドの演算`op`  
・モノイドの単位元`e`  
を定義する必要があります．
### 制約
* `T`は非負整数型．  
* `op`は結合律を満たす二項演算．  
* `e`は`op`の単位元．  
### 計算量
* O(1)
<br>
## set
```cpp
void set(T v, const S& x)
```
キー`v`に対応する値を`x`に変更します．
### 制約
* `0 <= v`  
### 計算量
* O(log(V_max))  
（`V_max`はキーの最大値）
<br>
## get
```cpp
S get(T p) const
```
キー`p`に対応する値を取得します．
### 制約
* `0 <= p`
### 計算量
* O(log(V_max))  
（`V_max`はキーの最大値）
<br>
## prod
```cpp
S prod(T l, T r) const
```
キーが`l`以上`r`未満の範囲にある値の列に対して，`op`を適用した結果を返します．
### 制約
* `0 <= l <= r`
### 計算量
* O(log(V_max))  
（`V_max`はキーの最大値）
<br>
## all_prod
```cpp
S all_prod() const
```
セグメント木全体に格納されているすべての値に対して，`op`を適用した結果を返します．
### 制約
* なし
### 計算量
* O(log(V_max))
<br>
## reserve
```cpp
void reserve(int n)
```
内部のノード格納用メモリを事前に確保します．
### 制約
* `n`は確保したい要素数．  
### 計算量
* O(n)