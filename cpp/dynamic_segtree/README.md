# dynamic segtree

動的セグ木（必要なところだけ作るセグ木）です。  
・一点更新  
・区間積  
だけ行えます。  
  
空間 $O(n)$ です
<br>

## コンストラクタ
```cpp
dynamic_segtree<T, S, op, e> seg;
```

T は添え字の型  
S・op・eはACLと同じ  
<br>

## set(T p, S x)
$O(log N)$  

## get(T p)
$O(log N)$  

## prod(T l, T r)
$O(log N)$  

## all_prod()
$O(log N)$  

## reserve

```cpp
void reserve(int n)
```

**・ 概要 :** 合計n回setしたときに、内部のvectorで再配置が起こらないようにする  
**・ 制約 :** 0 ≤ n < 常識  
**・ (空間)計算量 :** $O(n)$  
<br>

verified? : [Library Checker Point Set Range Composite (Large Array)](https://judge.yosupo.jp/submission/286197)
