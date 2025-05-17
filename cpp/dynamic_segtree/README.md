# dynamic segtree

動的セグ木（必要なところだけ作るセグ木）です。  
・一点更新  
・区間積  
だけ行えます。  
<br>

## コンストラクタ
```cpp
dynamic_segtree<T, S, op, e> seg;
```

T は添え字の型  
S・op・eはACLと同じ  
<br>

## set
はい  

## prod
はい  

## reserve

```cpp
void reserve(int n)
```

**・ 概要 :** 合計n回setしたときに、内部のvectorで再配置が起こらないようにする  
**・ 制約 :** 0 ≤ n < 常識  
**・ (空間)計算量 :** Θ(n)  
<br>

verified? : [Library Checker Point Set Range Composite (Large Array)](https://judge.yosupo.jp/submission/286197)
