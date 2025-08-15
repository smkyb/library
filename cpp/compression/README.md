# compression
座標圧縮をします。  
複数のvectorを同一視して処理できます*1
```cpp
vector<T> compression(T& v1, Args&... args)
```
座標圧縮をし、要素一覧(unique, sorted)を返します．
### 制約
型が一致する 
## 計算量
O(N log N)  
<br><br>
*1  
{1, 2, 3, 4, 5, 4}, {2, 4, 6, 8, 10, 2}  
が  
{0, 1, 2, 3, 4, 3}, {1, 3, 5, 6, 7, 1}  
になる
