#include "../meldable_binary_trie/meldable_binary_trie.cpp"
#include "../fastset/fastset.cpp"
#include "../segtree/segtree.cpp"
template<typename T, typename S, auto op, auto e>
struct sort_segtree {
    struct SS {
        S ltor, rtol;
        SS() = default;
        SS(const S &o) : ltor(o), rtol(o) {}
        SS(const S &l, const S &r) : ltor(l), rtol(r) {}
        static SS SS_op(const SS &l, const SS &r) {return SS{op(l.ltor, r.ltor), op(r.rtol, l.rtol)};}
        static SS SS_e() {return SS{e()};}
    };
    
    int n;
    meldable_binary_trie<T, SS, SS::SS_op, SS::SS_e> *trie;
    fastset fset;
    segtree<S, op, e> seg;
    
    sort_segtree(int _n) : n(_n), fset(n+1), seg(n) {
        trie = new meldable_binary_trie<T, SS, SS::SS_op, SS::SS_e>[n]{};
        for(int i = 0; i <= n; i++) fset.insert(i);
    }
    sort_segtree(const vector<pair<T, S>> &v) : n(v.size()), fset(n+1), seg(n) {
        trie = new meldable_binary_trie<T, SS, SS::SS_op, SS::SS_e>[n];
        for(int i = 0; i <= n; i++) fset.insert(i);
        for(int i = 0; i < n; i++) trie[i] = meldable_binary_trie<T, SS, SS::SS_op, SS::SS_e>(v[i].first, v[i].second);
        for(int i = 0; i < n; i++) seg.set(i, v[i].second);
    }
    sort_segtree(sort_segtree<T, S, op, e> &&o) = default;
    
    void set(int i, T k, S x){
        int l = fset.less_bound(i);
        int r = fset.lower_bound(i+1);
        if(l < i){
            auto res = trie[l].split(i-l);
            trie[l] = move(res.first); trie[i] = move(res.second);
            fset.insert(i);
            seg.set(l, trie[l].rev ? trie[l].all_prod().rtol : trie[l].all_prod().ltor);
        }
        if(i+1 < r){
            auto res = trie[i].split_one();
            trie[i+1] = move(res);
            fset.insert(i+1);
            seg.set(i+1, trie[i+1].rev ? trie[i+1].all_prod().rtol : trie[i+1].all_prod().ltor);
        }
        trie[i].init(k, x);
        seg.set(i, trie[i].rev ? trie[i].all_prod().rtol : trie[i].all_prod().ltor);
    }
    
    S prod(int l, int r) {
        int lb = fset.less_bound(l);
        if(lb < l){
            auto res = trie[lb].split(l-lb);
            trie[lb] = move(res.first); trie[l] = move(res.second);
            fset.insert(l);
            seg.set(lb, trie[lb].rev ? trie[lb].all_prod().rtol : trie[lb].all_prod().ltor);
            seg.set(l, trie[l].rev ? trie[l].all_prod().rtol : trie[l].all_prod().ltor);
        }
        int rb = fset.less_bound(r);
        if(rb < r){
            auto res = trie[rb].split(r-rb);
            trie[rb] = move(res.first); trie[r] = move(res.second);
            fset.insert(r);
            seg.set(rb, trie[rb].rev ? trie[rb].all_prod().rtol : trie[rb].all_prod().ltor);
            seg.set(r, trie[r].rev ? trie[r].all_prod().rtol : trie[r].all_prod().ltor);
        }
        return seg.prod(l, r);
    }
    
    void sort(int l, int r) {
        int lb = fset.less_bound(l);
        if(lb < l){
            auto res = trie[lb].split(l-lb);
            trie[lb] = move(res.first); trie[l] = move(res.second);
            fset.insert(l);
            seg.set(lb, trie[lb].rev ? trie[lb].all_prod().rtol : trie[lb].all_prod().ltor);
        }
        int rb = fset.less_bound(r);
        if(rb < r){
            auto res = trie[rb].split(r-rb);
            trie[rb] = move(res.first); trie[r] = move(res.second);
            fset.insert(r);
            seg.set(r, trie[r].rev ? trie[r].all_prod().rtol : trie[r].all_prod().ltor);
        }
        lb = fset.lower_bound(l+1);
        while(lb < r){
            auto res = meldable_binary_trie<T, SS, SS::SS_op, SS::SS_e>::meld(trie[l], trie[lb]);
            trie[l] = meldable_binary_trie<T, SS, SS::SS_op, SS::SS_e>(res);
            fset.erase(lb);
            seg.set(lb, e());
            lb = fset.lower_bound(lb);
        }
        trie[l].rev = false;
        seg.set(l, trie[l].all_prod().ltor);
    }
    
    void sort_rev(int l, int r) {
        int lb = fset.less_bound(l);
        if(lb < l){
            auto res = trie[lb].split(l-lb);
            trie[lb] = move(res.first); trie[l] = move(res.second);
            fset.insert(l);
            seg.set(lb, trie[lb].rev ? trie[lb].all_prod().rtol : trie[lb].all_prod().ltor);
        }
        int rb = fset.less_bound(r);
        if(rb < r){
            auto res = trie[rb].split(r-rb);
            trie[rb] = move(res.first); trie[r] = move(res.second);
            fset.insert(r);
            seg.set(r, trie[r].rev ? trie[r].all_prod().rtol : trie[r].all_prod().ltor);
        }
        lb = fset.lower_bound(l+1);
        while(lb < r){
            auto res = meldable_binary_trie<T, SS, SS::SS_op, SS::SS_e>::meld(trie[l], trie[lb]);
            trie[l] = meldable_binary_trie<T, SS, SS::SS_op, SS::SS_e>(res);
            fset.erase(lb);
            seg.set(lb, e());
            lb = fset.lower_bound(lb);
        }
        trie[l].rev = true;
        seg.set(l, trie[l].all_prod().rtol);
    }
};