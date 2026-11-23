// 注意: 整数型 T (sizeof(T) <= 8、bool 不可)・S はデフォルト構築可能・C++20 (GCC/Clang) 専用; ノード数上限 2^32-1
//       prod(l, r) は半開区間 [l, r) のため T の最大値は含められない (全体は all_prod() を使用)
//       reserve(n) は n >= 0 必須
template <typename T, typename S, auto op, auto e>
struct dynamic_segtree{
    static_assert(is_integral_v<T> && !is_same_v<T, bool> && sizeof(T) <= 8);
    
    private:
    
    static constexpr T T_MAX = numeric_limits<T>::max();
    static constexpr int B_L = sizeof(S) <= 8 ? 16 : 8;
    static constexpr int B_I = 16;
    
    struct Leaf{
        T key[B_L];
        S val[B_L];
        int sz = 0;
        
        Leaf(){
            for(int i = 0; i < B_L; ++i) key[i] = T_MAX;
        }
        
        inline int lower_bound(T p) const noexcept {
            int idx = 0;
            for(int i = 0; i < B_L; ++i) idx += (key[i] < p);
            return idx;
        }
        
        inline int upper_bound(T p) const noexcept {
            int idx = 0;
            for(int i = 0; i < B_L; ++i) idx += (key[i] <= p);
            return idx < sz ? idx : sz;
        }
        
        inline S fold_all() const {
            S s = val[0];
            for(int i = 1; i < sz; ++i) s = op(s, val[i]);
            return s;
        }
        
        inline S fold_range(int l, int r) const {
            if(l >= r) return e();
            S s = val[l];
            for(int i = l + 1; i < r; ++i) s = op(s, val[i]);
            return s;
        }
    };
    
    struct Internal{
        T sep[B_I];
        uint32_t child[B_I];
        int sz = 0;
        S tree[B_I * 2];
        
        Internal(){
            for(int i = 0; i < B_I; ++i) sep[i] = T_MAX;
            for(int i = 0; i < B_I * 2; ++i) tree[i] = e();
        }
        
        inline int find_child(T p) const noexcept {
            int idx = 0;
            for(int i = 0; i < B_I; ++i) idx += (sep[i] < p);
            return idx;
        }
        
        inline S update_slot(int idx, const S &v){
            int k = B_I + idx;
            tree[k] = v;
            k >>= 1; tree[k] = op(tree[k << 1], tree[(k << 1) | 1]);
            k >>= 1; tree[k] = op(tree[k << 1], tree[(k << 1) | 1]);
            k >>= 1; tree[k] = op(tree[k << 1], tree[(k << 1) | 1]);
            k >>= 1; tree[k] = op(tree[k << 1], tree[(k << 1) | 1]);
            return tree[1];
        }
        
        inline void rebuild(){
            for(int i = sz; i < B_I; ++i) tree[B_I + i] = e();
            for(int i = B_I - 1; i >= 1; --i) tree[i] = op(tree[i << 1], tree[(i << 1) | 1]);
        }
        
        inline S fold_range(int l, int r) const {
            if(l >= r) return e();
            if(l + 1 == r) return tree[B_I + l];
            if(l == 0 && r == sz) return tree[1];
            S la = e(), ra = e();
            bool hl = false, hr = false;
            for(l += B_I, r += B_I; l < r; l >>= 1, r >>= 1){
                if(l & 1){ la = hl ? op(la, tree[l++]) : tree[l++]; hl = true; }
                if(r & 1){ ra = hr ? op(tree[--r], ra) : tree[--r]; hr = true; }
            }
            if(!hl) return ra;
            if(!hr) return la;
            return op(la, ra);
        }
        
        inline S fold_suffix(int l) const {
            if(l >= sz) return e();
            if(l + 1 == sz) return tree[B_I + l];
            S res = e();
            bool has = false;
            for(int k = B_I + l, r = B_I * 2; k < r; k >>= 1, r >>= 1){
                if(k & 1){ res = has ? op(res, tree[k++]) : tree[k++]; has = true; }
            }
            return res;
        }
        
        inline S fold_prefix(int r) const {
            if(r <= 0) return e();
            if(r == 1) return tree[B_I];
            S res = e();
            bool has = false;
            for(int l = B_I, k = B_I + r; l < k; l >>= 1, k >>= 1){
                if(k & 1){ res = has ? op(tree[--k], res) : tree[--k]; has = true; }
            }
            return res;
        }
    };
    
    vector<Leaf> leaves;
    vector<Internal> internals;
    uint32_t root = 0;
    int height = 0;
    S total = e();
    
    template<bool is_add>
    void _update(T p, const S &x){
        if(height == 0) [[unlikely]] {
            leaves.emplace_back();
            Leaf &lf = leaves[0];
            lf.key[0] = p;
            lf.val[0] = x;
            lf.sz = 1;
            root = 0;
            height = 1;
            total = x;
            return;
        }
        uint32_t path_id[16];
        int path_idx[16];
        uint32_t cur = root;
        for(int h = height - 1; h >= 1; --h){
            const Internal &nd = internals[cur];
            int idx = nd.find_child(p);
            path_id[h] = cur;
            path_idx[h] = idx;
            uint32_t nxt = nd.child[idx];
            __builtin_prefetch(&nd.tree[B_I + idx], 1, 3);
            __builtin_prefetch(&nd.tree[1], 1, 3);
            if(h == 1) __builtin_prefetch(&leaves[nxt], 1, 3);
            else __builtin_prefetch(&internals[nxt], 0, 3);
            cur = nxt;
        }
        
        Leaf &lf = leaves[cur];
        int pos = lf.lower_bound(p);
        if(pos < lf.sz && lf.key[pos] == p){
            if constexpr (is_add) lf.val[pos] = op(lf.val[pos], x);
            else lf.val[pos] = x;
            S cur_sum = lf.fold_all();
            for(int h = 1; h < height; ++h){
                cur_sum = internals[path_id[h]].update_slot(path_idx[h], cur_sum);
            }
            total = cur_sum;
            return;
        }
        
        bool did_split = false;
        T split_key = 0;
        uint32_t new_child = 0;
        S left_sum, right_sum;
        
        if(lf.sz < B_L) [[likely]] {
            int nshift = lf.sz - pos;
            if(nshift > 0){
                memmove(lf.key + pos + 1, lf.key + pos, static_cast<size_t>(nshift) * sizeof(T));
                if constexpr (is_trivially_copyable_v<S>){
                    memmove(lf.val + pos + 1, lf.val + pos, static_cast<size_t>(nshift) * sizeof(S));
                } else {
                    for(int i = lf.sz; i > pos; --i) lf.val[i] = move(lf.val[i - 1]);
                }
            }
            lf.key[pos] = p;
            lf.val[pos] = x;
            lf.sz++;
            left_sum = lf.fold_all();
        } else {
            uint32_t r_id = static_cast<uint32_t>(leaves.size());
            leaves.emplace_back();
            Leaf &l = leaves[cur];
            Leaf &r = leaves[r_id];
            if(pos == B_L){
                r.key[0] = p;
                r.val[0] = x;
                r.sz = 1;
                split_key = l.key[B_L - 1];
                left_sum = (height > 1) ? internals[path_id[1]].tree[B_I + path_idx[1]] : total;
                right_sum = x;
            } else {
                constexpr int HALF = B_L / 2;
                if(pos < HALF){
                    for(int i = 0; i < B_L - HALF + 1; ++i){
                        r.key[i] = l.key[HALF - 1 + i];
                        r.val[i] = move(l.val[HALF - 1 + i]);
                    }
                    r.sz = B_L - HALF + 1;
                    for(int i = HALF - 1; i > pos; --i){
                        l.key[i] = l.key[i - 1];
                        l.val[i] = move(l.val[i - 1]);
                    }
                    l.key[pos] = p;
                    l.val[pos] = x;
                } else {
                    int rpos = pos - HALF;
                    for(int i = 0; i < rpos; ++i){
                        r.key[i] = l.key[HALF + i];
                        r.val[i] = move(l.val[HALF + i]);
                    }
                    r.key[rpos] = p;
                    r.val[rpos] = x;
                    for(int i = rpos; i < B_L - HALF; ++i){
                        r.key[i + 1] = l.key[HALF + i];
                        r.val[i + 1] = move(l.val[HALF + i]);
                    }
                    r.sz = B_L - HALF + 1;
                }
                for(int i = HALF; i < B_L; ++i) l.key[i] = T_MAX;
                l.sz = HALF;
                split_key = l.key[HALF - 1];
                left_sum = l.fold_all();
                right_sum = r.fold_all();
            }
            new_child = r_id;
            did_split = true;
        }
        
        for(int h = 1; h < height; ++h){
            uint32_t iid = path_id[h];
            int idx = path_idx[h];
            if(!did_split) [[likely]] {
                left_sum = internals[iid].update_slot(idx, left_sum);
            } else {
                Internal &nd = internals[iid];
                if(nd.sz < B_I){
                    for(int i = nd.sz - 1; i > idx; --i) nd.sep[i] = nd.sep[i - 1];
                    nd.sep[idx] = split_key;
                    for(int i = nd.sz; i > idx + 1; --i){
                        nd.child[i] = nd.child[i - 1];
                        nd.tree[B_I + i] = move(nd.tree[B_I + i - 1]);
                    }
                    nd.child[idx + 1] = new_child;
                    nd.tree[B_I + idx] = move(left_sum);
                    nd.tree[B_I + idx + 1] = move(right_sum);
                    nd.sz++;
                    nd.rebuild();
                    left_sum = nd.tree[1];
                    did_split = false;
                } else {
                    uint32_t r_id = static_cast<uint32_t>(internals.size());
                    internals.emplace_back();
                    Internal &l = internals[iid];
                    Internal &r = internals[r_id];
                    T tmp_sep[B_I];
                    uint32_t tmp_ch[B_I + 1];
                    S tmp_sum[B_I + 1];
                    for(int i = 0; i < idx; ++i){
                        tmp_sep[i] = l.sep[i];
                        tmp_ch[i] = l.child[i];
                        tmp_sum[i] = move(l.tree[B_I + i]);
                    }
                    tmp_ch[idx] = l.child[idx];
                    tmp_sum[idx] = move(left_sum);
                    tmp_sep[idx] = split_key;
                    tmp_ch[idx + 1] = new_child;
                    tmp_sum[idx + 1] = move(right_sum);
                    for(int i = idx + 1; i < B_I; ++i){
                        tmp_sep[i] = l.sep[i - 1];
                        tmp_ch[i + 1] = l.child[i];
                        tmp_sum[i + 1] = move(l.tree[B_I + i]);
                    }
                    constexpr int HALF = B_I / 2;
                    l.sz = HALF;
                    for(int i = 0; i < HALF; ++i){
                        l.child[i] = tmp_ch[i];
                        l.tree[B_I + i] = move(tmp_sum[i]);
                    }
                    for(int i = 0; i < HALF - 1; ++i) l.sep[i] = tmp_sep[i];
                    for(int i = HALF - 1; i < B_I; ++i) l.sep[i] = T_MAX;
                    l.rebuild();
                    
                    split_key = tmp_sep[HALF - 1];
                    
                    r.sz = B_I + 1 - HALF;
                    for(int i = 0; i < r.sz; ++i){
                        r.child[i] = tmp_ch[HALF + i];
                        r.tree[B_I + i] = move(tmp_sum[HALF + i]);
                    }
                    for(int i = 0; i < r.sz - 1; ++i) r.sep[i] = tmp_sep[HALF + i];
                    r.rebuild();
                    
                    left_sum = l.tree[1];
                    right_sum = r.tree[1];
                    new_child = r_id;
                }
            }
        }
        
        if(did_split) [[unlikely]] {
            uint32_t nroot = static_cast<uint32_t>(internals.size());
            internals.emplace_back();
            Internal &rt = internals[nroot];
            rt.sz = 2;
            rt.sep[0] = split_key;
            rt.child[0] = root;
            rt.child[1] = new_child;
            rt.tree[B_I + 0] = move(left_sum);
            rt.tree[B_I + 1] = move(right_sum);
            rt.rebuild();
            root = nroot;
            height++;
            total = rt.tree[1];
        } else {
            total = left_sum;
        }
    }
    
    public:
    
    dynamic_segtree() = default;
    dynamic_segtree(const dynamic_segtree&) = default;
    dynamic_segtree& operator=(const dynamic_segtree&) = default;
    dynamic_segtree(dynamic_segtree&&) = default;
    dynamic_segtree& operator=(dynamic_segtree&&) = default;
    
    void set(T p, const S &x) { _update<false>(p, x); }
    
    void add(T p, const S &x) { _update<true>(p, x); }
    
    S get(T p) const {
        if(height == 0) return e();
        uint32_t cur = root;
        for(int h = height - 1; h >= 1; --h){
            const Internal &nd = internals[cur];
            cur = nd.child[nd.find_child(p)];
        }
        const Leaf &lf = leaves[cur];
        int pos = lf.lower_bound(p);
        return (pos < lf.sz && lf.key[pos] == p) ? lf.val[pos] : e();
    }
    
    S prod(T l, T r) const {
        if(l >= r || height == 0) return e();
        T R = r - 1;
        uint32_t cur = root;
        int h = height - 1;
        while(h >= 1){
            const Internal &nd = internals[cur];
            int il = nd.find_child(l);
            int ir = nd.find_child(R);
            if(il == ir){
                cur = nd.child[il];
                --h;
                continue;
            }
            uint32_t lc = nd.child[il];
            uint32_t rc = nd.child[ir];
            if(h == 1){
                __builtin_prefetch(&leaves[lc], 0, 3);
                __builtin_prefetch(&leaves[rc], 0, 3);
            } else {
                __builtin_prefetch(&internals[lc], 0, 3);
                __builtin_prefetch(&internals[rc], 0, 3);
            }
            bool has_mid = (il + 1 < ir);
            S mid_val = has_mid ? nd.fold_range(il + 1, ir) : e();
            
            S lans = e(), rans = e();
            bool hl = false, hr = false;
            for(int dh = h - 1; dh >= 1; --dh){
                const Internal &lnd = internals[lc];
                const Internal &rnd = internals[rc];
                int li = lnd.find_child(l);
                int ri = rnd.find_child(R);
                lc = lnd.child[li];
                rc = rnd.child[ri];
                if(dh == 1){
                    __builtin_prefetch(&leaves[lc], 0, 3);
                    __builtin_prefetch(&leaves[rc], 0, 3);
                } else {
                    __builtin_prefetch(&internals[lc], 0, 3);
                    __builtin_prefetch(&internals[rc], 0, 3);
                }
                if(li + 1 < lnd.sz){
                    S suf = lnd.fold_suffix(li + 1);
                    lans = hl ? op(suf, lans) : suf;
                    hl = true;
                }
                if(ri > 0){
                    S pre = rnd.fold_prefix(ri);
                    rans = hr ? op(rans, pre) : pre;
                    hr = true;
                }
            }
            const Leaf &llf = leaves[lc];
            const Leaf &rlf = leaves[rc];
            int lp = llf.lower_bound(l);
            int rp = rlf.upper_bound(R);
            if(lp < llf.sz){
                S s = llf.fold_range(lp, llf.sz);
                lans = hl ? op(s, lans) : s;
                hl = true;
            }
            if(has_mid){
                lans = hl ? op(lans, mid_val) : mid_val;
                hl = true;
            }
            if(rp > 0){
                S s = rlf.fold_range(0, rp);
                rans = hr ? op(rans, s) : s;
                hr = true;
            }
            if(!hl) return rans;
            if(!hr) return lans;
            return op(lans, rans);
        }
        const Leaf &lf = leaves[cur];
        int lp = lf.lower_bound(l);
        int rp = lf.upper_bound(R);
        return lf.fold_range(lp, rp);
    }
    
    S all_prod() const {
        return total;
    }
    
    void reserve(int n){
        assert(n >= 0);
        if(n == 0) return;
        size_t nl = static_cast<size_t>(n) / (B_L / 2) + 4;
        leaves.reserve(nl);
        if(n > B_L) internals.reserve(nl / (B_I / 2) + 8);
    }
};
