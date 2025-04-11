void _compression_merge(...){}
template<typename T, typename... Args>
void _compression_merge(vector<T>& C, vector<T>& v, Args&... args){
    for(auto& i : v) C.push_back(i);
    _compression_merge(C, args...);
}
void _compression_apply(...){}
template<typename T, typename... Args>
void _compression_apply(vector<T>& C, vector<T>& v, Args&... args){
    for(auto& i : v) i = lower_bound(C.begin(), C.end(), i) - C.begin();
    _compression_apply(C, args...);
}
template<typename T, typename... Args>
vector<T> compression(vector<T>& v, Args&... args){
    vector<T> C;
    _compression_merge(C, v, args...);
    sort(C.begin(), C.end());
    C.erase(unique(C.begin(), C.end()), C.end());
    _compression_apply(C, v, args...);
    return C;
}