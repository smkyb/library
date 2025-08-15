# fastset

高速なsetです。

## コンストラクタ

```cpp
fastset(int _n);
```

・ 最大値が`_n`であるsetを構築します。  
・ 内部的には64bitのunsigned long long型を用いてビット管理を行います。

### 制約

・ `_n` が正
### 計算量
・ O(n)

## insert

```cpp
void insert(int x);
```

setに要素`x`を追加します。

### 制約

・ `0 <= x < _n`
### 計算量
・ O(1)

## erase

```cpp
void erase(int x);
```

setから要素`x`を削除します。

### 制約

・ `0 <= x < _n`
### 計算量
・ O(1)

## count

```cpp
bool count(int x) const;
```

setに要素`x`が含まれているかどうかを判定します。

### 制約

・ `0 <= x < _n`
### 計算量
・ O(1)

## lower_bound

```cpp
int lower_bound(int x);
```

`x`以上の最小の要素を返します。そのような要素がない場合は-1を返します。

### 制約

・ `0 <= x < _n`
### 計算量
・ O(log(_n))

## less_bound

```cpp
int less_bound(int x);
```

`x`未満の最大の要素を返します。そのような要素がない場合は-1を返します。

### 制約

・ `0 <= x < _n`
### 計算量
・ O(log(_n))
