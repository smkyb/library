template<typename S, auto op, auto e>
struct segtree{
    int siz = 0;
    vector<S> node;
    
    segtree() = default;
    segtree(unsigned n):
        siz(bit_ceil(n)), node(siz*2, e()){}
    segtree(const vector<S>& v){init(v);}
    void init(const vector<S>& v){
        siz = bit_ceil(v.size()); node.assign(siz*2, e());
        for(int i = 0; i < (int)v.size(); i++) node[i+siz] = v[i];
        for(int i = siz-1; i >= 1; i--) node[i] = op(node[i<<1], node[(i<<1)|1]);
    }
    
    typename vector<S>::const_reference operator[](int p) const {return node[p+siz];}
    
    typename vector<S>::const_reference get(int p) const {return (*this)[p];}
    
    void set(int p, const S &x){
        assert(0 <= p && p < siz);
        p += siz;
        node[p] = x;
        while(p>>=1) node[p] = op(node[p<<1], node[(p<<1)|1]);
    }
    
    void add(int p, const S &x){
        assert(0 <= p && p < siz);
        p += siz;
        node[p] = op(node[p], x);
        while(p>>=1) node[p] = op(node[p<<1], node[(p<<1)|1]);
    }
    
    S prod(int l, int r) const {
        l = max(0, l); r = min(r, siz);
        if(l >= r) return e();
        if(l + 1 == r) return node[l + siz];
        S l_ans = e(), r_ans = e();
        bool has_l = false, has_r = false;
        for(l+=siz, r+=siz; l < r; l>>=1, r>>=1){
            if(l&1){
                l_ans = has_l ? op(l_ans, node[l++]) : node[l++];
                has_l = true;
            }
            if(r&1){
                r_ans = has_r ? op(node[--r], r_ans) : node[--r];
                has_r = true;
            }
        }
        if(!has_l) return r_ans;
        if(!has_r) return l_ans;
        return op(l_ans, r_ans);
    }
    
    S all_prod() const {
        return node.empty() ? e() : node[1];
    }
};
