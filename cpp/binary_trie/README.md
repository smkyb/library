# binary trie
非負整数の多重集合で、  
・要素の挿入、削除、検索  
・xor最小、最大値の取得  
・k番目の要素の取得  
・k以上の最小、k以下の最大の値の取得  
・k以下の要素の個数の取得  
を行えます。（全体xor作用は実装していません）  
<br>
## コンストラクタ
```cpp
binary_trie<T> bin
```
Tは符号付き整数型が好ましいです（範囲外で-1を返すため）。   
<br>
## insert
```cpp
void insert(T x)
```
**・ 概要 :** 要素の挿入  
**・ 制約 :** 0 <= x  
**・ 計算量 :** Θ(bit)  
<br>
## erase
```cpp
void erase(T x, int n = inf)
```
**・ 概要 :** 要素の削除  
**・ 制約 :** 0 <= x , 0 <= n <= count(x)  
**・ 計算量 :** Θ(bit)  
<br>
## count
```cpp
int count(T x)
```
**・ 概要 :** 要素の個数の取得  
**・ 制約 :** 0 <= x  
**・ 計算量 :** Θ(bit)  
<br>
## xor_min / xor_max
```cpp
T xor_min(T x)
T xor_max(T x)
```
**・ 概要 :** xor最小、最大値の取得（xorされたものが返ってくる）  
**・ 制約 :** 0 <= x  
**・ 計算量 :** Θ(bit)  
<br>
## get_smallest / get_largest
```cpp
T get_smallest(int k)
T get_largest(int k)
```
**・ 概要 :** k番目の値の取得  
**・ 制約 :** 0 <= k < size()  
**・ 計算量 :** Θ(bit)  
<br>
## lower_bound / less_bound
```cpp
T lower_bound(T x)
T less_bound(T x)
```
**・ 概要 :** x以上で最小、x以下で最大の値の取得。なければ-1を返す  
**・ 制約 :** 0 <= x  
**・ 計算量 :** Θ(bit) 
<br>
## order
```cpp
int order(T x)
```
**・ 概要 :** x以下の要素の個数の取得  
**・ 制約 :** 0 <= x  
**・ 計算量 :** Θ(bit)   
<br>
## reserve
```cpp
void reserve(int n)
```
**・ 概要 :** n回insertした場合にvectorの再確保が行われないようにreserveする  
**・ 制約 :** 0 <= n < 常識  
**・ 計算量 :** Θ(n)  
<br>
## size
```cpp
void size()
```
**・ 概要 :** 要素数の取得
**・ 計算量 :** Θ(1)  
