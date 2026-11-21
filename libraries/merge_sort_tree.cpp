// 注意: 静的 (点更新なし); count_and_sum_leq(l, r, x) は半開 [l, r) で !comp(x, a[i]) (既定: a[i] <= x) の {個数, 総和} を返す
//       総和は T 型で累積されるためオーバーフロー注意 (T(0) と + が必要)
template<typename T, auto comp = less<T>()>
struct merge_sort_tree{
    int siz = 0;
    int h = 0;
    vector<T> vals, pref;
    vector<int> lcnt;
    
    merge_sort_tree() = default;
    merge_sort_tree(const vector<T>& v) : siz(static_cast<int>(v.size())) {
        if(siz == 0) return;
        h = (siz <= 1) ? 1 : (32 - __builtin_clz(static_cast<unsigned>(siz - 1))) + 1;
        size_t tot = static_cast<size_t>(h) * siz;
        vals.resize(tot);
        pref.resize(tot);
        lcnt.resize(tot);
        _build(0, 0, siz, v);
    }
    
    void _build(int d, int l, int r, const vector<T>& v){
        size_t off = static_cast<size_t>(d) * siz;
        if(r - l == 1){
            vals[off + l] = v[l];
            pref[off + l] = v[l];
            return;
        }
        int m = (l + r) >> 1;
        _build(d + 1, l, m, v);
        _build(d + 1, m, r, v);
        size_t coff = static_cast<size_t>(d + 1) * siz;
        int i = l, j = m;
        for(int k = l; k < r; ++k){
            if(j == r || (i < m && !comp(vals[coff + j], vals[coff + i]))){
                vals[off + k] = vals[coff + i++];
            } else {
                vals[off + k] = vals[coff + j++];
            }
            lcnt[off + k] = i - l;
            pref[off + k] = (k == l) ? vals[off + k] : (pref[off + k - 1] + vals[off + k]);
        }
    }
    
    pair<int, T> _query(int d, int l, int r, int ql, int qr, int cnt) const {
        if(cnt == 0 || qr <= l || r <= ql) return {0, T(0)};
        size_t off = static_cast<size_t>(d) * siz;
        if(ql <= l && r <= qr) return {cnt, pref[off + l + cnt - 1]};
        int m = (l + r) >> 1;
        int lc = lcnt[off + l + cnt - 1];
        int rc = cnt - lc;
        if(qr <= m) return _query(d + 1, l, m, ql, qr, lc);
        if(ql >= m) return _query(d + 1, m, r, ql, qr, rc);
        auto [lc_ans, ls_ans] = _query(d + 1, l, m, ql, qr, lc);
        auto [rc_ans, rs_ans] = _query(d + 1, m, r, ql, qr, rc);
        return {lc_ans + rc_ans, ls_ans + rs_ans};
    }
    
    pair<int, T> count_and_sum_leq(int l, int r, T x) const {
        assert(0 <= l && r <= siz);
        if(l >= r || siz == 0) return {0, T(0)};
        int cnt = static_cast<int>(upper_bound(vals.begin(), vals.begin() + siz, x, comp) - vals.begin());
        return _query(0, 0, siz, l, r, cnt);
    }
};