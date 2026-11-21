// 注意: 渡した全 vector<T> を破壊的に 0-indexed 順位へ書き換え、元値の昇順ユニーク配列を返す (同一の数値型 T 必須)
template<typename T>
void _compress_merge(vector<T>&){}
template<typename T, typename... Args>
void _compress_merge(vector<T>& C, vector<T>& v, Args&... args){
    for(auto& i : v) C.push_back(i);
    _compress_merge(C, args...);
}
template<typename T>
void _compress_apply(vector<T>&){}
template<typename T, typename... Args>
void _compress_apply(vector<T>& C, vector<T>& v, Args&... args){
    for(auto& i : v) i = lower_bound(C.begin(), C.end(), i) - C.begin();
    _compress_apply(C, args...);
}
template<typename T, typename... Args>
vector<T> compress(vector<T>& v, Args&... args){
    vector<T> C;
    _compress_merge(C, v, args...);
    sort(C.begin(), C.end());
    C.erase(unique(C.begin(), C.end()), C.end());
    _compress_apply(C, v, args...);
    return C;
}