// 注意: 値ではなく 0-indexed の添字列を返す (デフォルトは狭義単調増加)
template<typename T>
vector<int> lis(const vector<T>& v){
    //デフォルトで狭義単調増加
    int n = static_cast<int>(v.size());
    if(n == 0) return vector<int>{};
    vector<T> cur;
    vector<int> last_idx;
    vector<int> prev_idx(n, -1);
    cur.reserve(n);
    last_idx.reserve(n);
    for(int i = 0; i < n; ++i){
        auto itr = lower_bound(cur.begin(), cur.end(), v[i]);
        int pos = static_cast<int>(itr - cur.begin());
        if(itr == cur.end()){
            cur.push_back(v[i]);
            last_idx.push_back(i);
        } else {
            *itr = v[i];
            last_idx[pos] = i;
        }
        if(pos > 0) prev_idx[i] = last_idx[pos - 1];
    }
    int len = static_cast<int>(cur.size());
    vector<int> res(len);
    int idx = last_idx[len - 1];
    for(int i = len - 1; i >= 0; --i){
        res[i] = idx;
        idx = prev_idx[idx];
    }
    return res;
}