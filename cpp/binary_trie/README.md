# binary trie
非負整数の多重集合で、  
・要素の挿入、削除、検索  
・全体xor作用  
・xor最小、最大値の取得  
・k番目の要素の取得  
・k以上の最小、k以下の最大の値の取得  
・k以下の要素の個数の取得  
を行えます。  
<br>
## コンストラクタ
```cpp
binary_trie<T> bin
```
Tは整数  
<br>
## insert
```cpp
void insert(T x)
```
**・ 概要 :** 要素の挿入  
**・ 制約 :** 0 <= x  
**・ 計算量 :** O(bit)  
<br>
## erase
```cpp
void erase(T x, int n = inf)
```
**・ 概要 :** 要素の削除  
**・ 制約 :** 0 <= x , 0 <= n <= count(x)  
**・ 計算量 :** O(bit)  
<br>
## count
```cpp
int count(T x)
```
**・ 概要 :** 要素の個数の取得  
**・ 制約 :** 0 <= x  
**・ 計算量 :** O(bit)  
<br>
## operator[]
```cpp
ref_type operator[int x]
```
**・ 概要 :** k番目(0-indexed)の値の取得(負の数を引数にとると大きい方からk番目(1-indexed)を取得)  
**・ 制約 :** 0 <= k < size()  
**・ 計算量 :** O(bit)  
**・ 戻り値 :** .existで値の有無、.valで値の取得  
<br>
## xor_min / xor_max
```cpp
ref_type xor_min(T x)
ref_type xor_max(T x)
```
**・ 概要 :** xor最小、最大値の取得（xorされていない要素が返ってくる）  
**・ 制約 :** 0 <= x  
**・ 計算量 :** O(bit)  
**・ 戻り値 :** .existで値の有無、.valで値の取得  
<br>
## lower_bound / less_bound
```cpp
ref_type lower_bound(T x)
ref_type less_bound(T x)
```
**・ 概要 :** x以上で最小、x以下で最大の値の取得。なければ-1を返す  
**・ 制約 :** 0 <= x  
**・ 計算量 :** O(bit)  
**・ 戻り値 :** .existで値の有無、.valで値の取得  
<br>
## order
```cpp
int order(T x)
```
**・ 概要 :** x未満の要素の個数の取得  
**・ 制約 :** 0 <= x  
**・ 計算量 :** O(bit)   
<br>
## apply_xor
```cpp
void apply_xor(T x)
```
**・ 概要 :** 全体にxorを行う  
**・ 制約 :** 0 <= x  
**・ 計算量 :** O(1)  
<br>
## size
```cpp
void size()
```
**・ 概要 :** 要素数の取得
**・ 計算量 :** O(1)  
など
