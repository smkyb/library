// 注意: 頂点属性 (hld_v) / 辺属性 (hld_e) HLD (0-indexed・連結な木・C++20); add_edge をちょうど n-1 回呼んで build を 1 回だけ呼ぶこと
//       prod(u, v) / apply(u, v, x) は頂点属性なら両端を含む閉区間, 辺属性ならパス上の辺; dist(u, v) は重み和ではなく辺数
//       辺属性の set/add/get(u, v) は辺 (u, v) が存在する前提 (非チェック); 非可換時の apply は F が反転不変であること; node_type は空区間 prod(k, k)==e() 必須
template<typename node_type, typename S, auto op, auto e, bool is_edge, bool commutative>
struct _hld_base{
    struct Chain{
        int head, jump;
    };
    
    int n, r, edge_cnt;
    vector<int> in, out, rev_in, deg, adj_xor;
    vector<Chain> chain;
    vector<int> E;
    vector<S> V;
    
    node_type node[commutative ? 1 : 2];
    
    _hld_base(int n, int root = 0):
        n(n), r(root), edge_cnt(0),
        in(n), out(n), rev_in(n), deg(n, 0), adj_xor(n, 0), chain(n){
        if constexpr (is_edge){
            E.resize(max(0, n * 2 - 2));
            V.resize(max(0, n - 1));
        }
    }
    
    void add_edge(int u, int v) requires (!is_edge) {
        assert(0 <= u && u < n && 0 <= v && v < n && u != v);
        assert(edge_cnt < n - 1);
        deg[u]++; deg[v]++;
        adj_xor[u] ^= v; adj_xor[v] ^= u;
        edge_cnt++;
    }
    
    void add_edge(int u, int v, const S& x) requires (is_edge) {
        assert(0 <= u && u < n && 0 <= v && v < n && u != v);
        assert(edge_cnt < n - 1);
        deg[u]++; deg[v]++;
        adj_xor[u] ^= v; adj_xor[v] ^= u;
        V[edge_cnt] = x;
        E[edge_cnt << 1] = u;
        E[(edge_cnt << 1) | 1] = v;
        edge_cnt++;
    }
    
    void _build_tree(){
        assert(0 <= r && r < n);
        assert(edge_cnt == n - 1);
        
        vector<int> par(n, -1), sub_siz(n, 1), heavy(n, -1), heavy_sz(n, 0);
        vector<int> ord;
        ord.reserve(n);
        deg[r] = 0;
        for(int i = 0; i < n; ++i){
            int u = i;
            while(deg[u] == 1){
                int p = adj_xor[u];
                par[u] = p;
                ord.push_back(u);
                int su = sub_siz[u];
                sub_siz[p] += su;
                if(su > heavy_sz[p]){
                    heavy_sz[p] = su;
                    heavy[p] = u;
                }
                adj_xor[p] ^= u;
                --deg[u];
                --deg[p];
                u = p;
            }
        }
        assert((int)ord.size() == n - 1);
        vector<int>().swap(deg);
        vector<int>().swap(adj_xor);
        
        vector<int> cur_pos(n);
        in[r] = 0;
        out[r] = n;
        rev_in[0] = r;
        chain[0] = {0, -1};
        cur_pos[r] = 1 + (heavy[r] != -1 ? sub_siz[heavy[r]] : 0);
        for(int k = n - 2; k >= 0; --k){
            int u = ord[k];
            int p = par[u];
            int iu;
            if(u == heavy[p]){
                iu = in[p] + 1;
                chain[iu] = chain[in[p]];
            } else {
                iu = cur_pos[p];
                cur_pos[p] += sub_siz[u];
                chain[iu] = {iu, in[p]};
            }
            in[u] = iu;
            out[u] = iu + sub_siz[u];
            rev_in[iu] = u;
            cur_pos[u] = iu + 1 + (heavy[u] != -1 ? sub_siz[heavy[u]] : 0);
        }
    }
    
    void _build_node(vector<S>& node_v){
        node[0] = node_type(node_v);
        if constexpr (!commutative){
            reverse(node_v.begin(), node_v.end());
            node[1] = node_type(node_v);
        }
    }
    
    void build(const vector<S>& v) requires (!is_edge) {
        if(n == 0) return;
        assert((int)v.size() >= n);
        _build_tree();
        vector<S> node_v(n);
        for(int i = 0; i < n; ++i) node_v[in[i]] = v[i];
        _build_node(node_v);
    }
    
    void build() requires (is_edge) {
        if(n == 0) return;
        _build_tree();
        vector<S> node_v(n, e());
        for(int i = 0; i < n - 1; ++i){
            int u = E[i << 1], v = E[(i << 1) | 1];
            node_v[in[u] > in[v] ? in[u] : in[v]] = V[i];
        }
        vector<int>().swap(E);
        vector<S>().swap(V);
        _build_node(node_v);
    }
    
    S prod(int u, int v){
        assert(0 <= u && 0 <= v && u < n && v < n);
        if constexpr (is_edge){
            if(u == v) return e();
        }
        int l = in[u], r = in[v];
        S l_ans = e(), r_ans = e();
        bool has_l = false, has_r = false;
        auto prod_fwd = [&](int a, int b) -> S {
            if(a + 1 == b){
                if constexpr (requires(node_type& nt, int p) { nt.get(p); }) return node[0].get(a);
            }
            return node[0].prod(a, b);
        };
        auto prod_rev = [&](int a, int b) -> S {
            if(a + 1 == b){
                if constexpr (requires(node_type& nt, int p) { nt.get(p); }) return node[0].get(a);
            }
            if constexpr (commutative) return node[0].prod(a, b);
            else return node[1].prod(n - b, n - a);
        };
        while(chain[l].head != chain[r].head){
            if(l > r){
                S val = prod_rev(chain[l].head, l + 1);
                l_ans = has_l ? op(l_ans, val) : val;
                has_l = true;
                l = chain[l].jump;
            } else {
                S val = prod_fwd(chain[r].head, r + 1);
                r_ans = has_r ? op(val, r_ans) : val;
                has_r = true;
                r = chain[r].jump;
            }
        }
        constexpr int off = is_edge ? 1 : 0;
        if(l + off <= r){
            S val = prod_fwd(l + off, r + 1);
            l_ans = has_l ? op(l_ans, val) : val;
            has_l = true;
        } else if(r + off <= l){
            S val = prod_rev(r + off, l + 1);
            r_ans = has_r ? op(val, r_ans) : val;
            has_r = true;
        }
        if(!has_l) return r_ans;
        if(!has_r) return l_ans;
        return op(l_ans, r_ans);
    }
    
    S subtree_prod(int v){
        assert(0 <= v && v < n);
        constexpr int off = is_edge ? 1 : 0;
        return node[0].prod(in[v] + off, out[v]);
    }
    
    S get(int v) requires (!is_edge) {
        assert(0 <= v && v < n);
        return node[0].get(in[v]);
    }
    
    S get(int u, int v) requires (is_edge) {
        assert(0 <= u && u < n && 0 <= v && v < n && u != v);
        return node[0].get(max(in[u], in[v]));
    }
    
    void set(int v, const S& x) requires (!is_edge) {
        assert(0 <= v && v < n);
        int idx = in[v];
        node[0].set(idx, x);
        if constexpr (!commutative) node[1].set(n - 1 - idx, x);
    }
    
    void set(int u, int v, const S& x) requires (is_edge) {
        assert(0 <= u && u < n && 0 <= v && v < n && u != v);
        int idx = max(in[u], in[v]);
        node[0].set(idx, x);
        if constexpr (!commutative) node[1].set(n - 1 - idx, x);
    }
    
    void add(int v, const S& x) requires (!is_edge) {
        assert(0 <= v && v < n);
        int idx = in[v];
        node[0].add(idx, x);
        if constexpr (!commutative) node[1].add(n - 1 - idx, x);
    }
    
    void add(int u, int v, const S& x) requires (is_edge) {
        assert(0 <= u && u < n && 0 <= v && v < n && u != v);
        int idx = max(in[u], in[v]);
        node[0].add(idx, x);
        if constexpr (!commutative) node[1].add(n - 1 - idx, x);
    }
    
    int lca(int u, int v) const {
        assert(0 <= u && 0 <= v && u < n && v < n);
        int l = in[u], r = in[v];
        while(chain[l].head != chain[r].head){
            if(l > r) l = chain[l].jump;
            else r = chain[r].jump;
        }
        return rev_in[l < r ? l : r];
    }
    
    int dist(int u, int v) const {
        assert(0 <= u && 0 <= v && u < n && v < n);
        int l = in[u], r = in[v];
        int d = 0;
        while(chain[l].head != chain[r].head){
            if(l > r){
                d += l - chain[l].head + 1;
                l = chain[l].jump;
            } else {
                d += r - chain[r].head + 1;
                r = chain[r].jump;
            }
        }
        return d + abs(l - r);
    }
};

template<typename T, bool is_edge = false, bool commutative = false>
struct _hld_impl;
template<template<typename, auto, auto> typename node_type, typename S, auto op, auto e, bool is_edge, bool commutative>
struct _hld_impl<node_type<S, op, e>, is_edge, commutative> : _hld_base<node_type<S, op, e>, S, op, e, is_edge, commutative> {
    using Base = _hld_base<node_type<S, op, e>, S, op, e, is_edge, commutative>;
    using Base::Base;
};
template<template<typename, auto, auto, typename, auto, auto, auto> typename node_type, typename S, auto op, auto e, typename F, auto mapping, auto composition, auto id, bool is_edge, bool commutative>
struct _hld_impl<node_type<S, op, e, F, mapping, composition, id>, is_edge, commutative> : _hld_base<node_type<S, op, e, F, mapping, composition, id>, S, op, e, is_edge, commutative> {
    using Base = _hld_base<node_type<S, op, e, F, mapping, composition, id>, S, op, e, is_edge, commutative>;
    using Base::Base;
    using Base::n, Base::in, Base::out, Base::chain, Base::node;
    
    void apply(int u, int v, const F& x){
        assert(0 <= u && 0 <= v && u < n && v < n);
        if constexpr (is_edge){
            if(u == v) return;
        }
        int l = in[u], r = in[v];
        while(chain[l].head != chain[r].head){
            if(l < r) swap(l, r);
            int hl = chain[l].head;
            node[0].apply(hl, l + 1, x);
            if constexpr (!commutative) node[1].apply(n - l - 1, n - hl, x);
            l = chain[l].jump;
        }
        if(l > r) swap(l, r);
        constexpr int off = is_edge ? 1 : 0;
        if(l + off <= r){
            node[0].apply(l + off, r + 1, x);
            if constexpr (!commutative) node[1].apply(n - r - 1, n - l - off, x);
        }
    }
    
    void subtree_apply(int v, const F& x){
        assert(0 <= v && v < n);
        constexpr int off = is_edge ? 1 : 0;
        if(in[v] + off < out[v]){
            node[0].apply(in[v] + off, out[v], x);
            if constexpr (!commutative) node[1].apply(n - out[v], n - in[v] - off, x);
        }
    }
};

template<typename T, bool commutative = false>
using hld_v = _hld_impl<T, false, commutative>;
template<typename T, bool commutative = false>
using hld_e = _hld_impl<T, true, commutative>;
