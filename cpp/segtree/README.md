# segtree
セグメント木を実装したデータ構造です．  
モノイドの演算`op`と単位元`e`を定義して使用します．  
<br>
## コンストラクタ
```cpp
segtree(unsigned n);
segtree(const vector<S>& v);
```
・`S`: 値の型  
・`op`: モノイドの演算 (例: `[](S a, S b){ return a + b; }`)  
・`e`: モノイドの単位元 (例: `0`)  
<br>
・`segtree(unsigned n)`: 長さ`n`の数列のセグメント木を構築します．初期値はすべて単位元`e()`です．  
・`segtree(const vector<S>& v)`: 与えられた`vector<S>`のセグメント木を構築します．  
### 制約
・`n`は正であること．  
・`v`は空でないこと．  
### 計算量
・`segtree(unsigned n)`: O(n)  
・`segtree(const vector<S>& v)`: O(v.size())  
<br>
## init
```cpp
void init(const vector<S>& v);
```
与えられた`vector<S>`でセグメント木を初期化します．  
### 制約
・`v`は空でないこと．  
### 計算量
・O(v.size())  
<br>
## get
```cpp
const S &get(int pos) const;
```
`pos`番目の要素の値を取得します．  
### 制約
・`0 <= pos < siz` (`siz`はコンストラクタで指定された`n`以上の2のべき乗数)  
### 計算量
・O(1)  
<br>
## set
```cpp
void set(int pos, const S &x);
```
`pos`番目の要素の値を`x`に変更します．  
### 制約
・`0 <= pos < siz`  
### 計算量
・O(log(siz))  
<br>
## add
```cpp
void add(int pos, const S &x);
```
`pos`番目の要素の値に`x`を`op`で適用します．  
`node[pos+siz] = op(node[pos+siz], x)` を行います．  
### 制約
・`0 <= pos < siz`  
### 計算量
・O(log(siz))  
<br>
## prod
```cpp
S prod(int left, int right);
```
区間 `[left, right)` の要素に対するモノイド演算の結果を返します．  
`op`の適用順序は左から右へ行われます．  
### 制約
・`0 <= left <= right <= siz`  
### 計算量
・O(log(siz))  
<br>
## all_prod
```cpp
S all_prod();
```
セグメント木全体（すべての要素）に対するモノイド演算の結果を返します．  
### 制約
・なし  
### 計算量
・O(1)  
