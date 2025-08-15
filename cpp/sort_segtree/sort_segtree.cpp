namespace smkyb {

template<typename T, typename S, auto op, auto e>
struct meldable_binary_trie{
    static_assert(is_unsigned_v<T>);
    
    static constexpr T one = 1;
    static constexpr int bit_width = sizeof(T) * 8;
    
    struct node_t{
        T value;
        S sum;
        int width;
        int count;
        array<node_t*, 4> child;
        node_t() = default;
        node_t(T v, S s, int w, int c) : value(v), sum(s), width(w), count(c), child{&nil, &nil, &nil, &nil} {}
        node_t(T v, S s, int w, int c, node_t* c0, node_t* c1, node_t* c2, node_t* c3) : value(v), sum(s), width(w), count(c), child{c0, c1, c2, c3} {}
        static node_t nil;
    };
    
    struct ref_node_t{
        T val;
        bool exist;
        ref_node_t(T x, bool _e) : val{x}, exist{_e} {}
    };
    
    struct Pool {
        int cnt = 0;
        constexpr static int SIZ = 1<<17;
        node_t *ptr = nullptr, *en = nullptr;
        vector<node_t*> reuse;
        Pool() {
            ptr = new node_t[SIZ];
            en = ptr + SIZ;
        }
        node_t *get() {
            if(ptr == en) {
                if(!reuse.empty()) {
                    auto ptr = reuse.back();
                    reuse.pop_back();
                    return ptr;
                }
                ptr = new node_t[SIZ];
                en = ptr + SIZ;
            }
            return ptr++;
        }
        void push(node_t *p) {reuse.push_back(p);}
    };
    
    static Pool pool;
    node_t* root;
    bool rev = false;
    
    template<typename _Tp>
    inline static int clz(_Tp x) {
        if constexpr(sizeof(_Tp) == 8ull) return __builtin_clzll(x);
        else return __builtin_clz(x);
    }
    
    inline static T mask(int l, int r) {
        if(r >= bit_width) return -(one<<l);
        return (one<<r) - (one<<l);
    }
    
    inline static T masked(T v, int l, int r) {
        return mask(l, r) & v;
    }
    
    inline static int diff_bit(T x, T y) {
        return ((bit_width-1 - clz(x^y))|1) + 1;
    }
    
    //value, sum, width, count, child
    template<typename... Args>
    static inline node_t* make_node(Args ...args) {
        return &((*pool.get()) = node_t(forward<Args>(args)...));
    }
    
    public:
    
    meldable_binary_trie(){
        root = make_node(0, e(), bit_width, 0);
    }
    meldable_binary_trie(T p, const S &x){
        root = make_node(p, x, bit_width, 1);
    }
    meldable_binary_trie(node_t *x){
        root = x;
    }
    
    meldable_binary_trie& operator=(meldable_binary_trie&& o) noexcept = default;
    
    meldable_binary_trie(meldable_binary_trie&& o) noexcept = default;
    
    void insert(T v, const S &x) {
        node_t* pos = root;
        int bit = bit_width;
        node_t *route[65]; int route_cnt = 0;
        while(true){
            T mv = masked(v, bit-pos->width, bit);
            T mnv = masked(pos->value, bit-pos->width, bit);
            if(mv != mnv){
                route[route_cnt++] = pos;
                int diff = diff_bit(mv, mnv);
                int b = (mv>>(diff-2))&3;
                int nb = (mnv>>(diff-2))&3;
                node_t* inter = make_node(pos->value, pos->sum, pos->width-bit+diff, pos->count, pos->child[0], pos->child[1], pos->child[2], pos->child[3]);
                node_t* leaf = make_node(v, x, diff, 1);
                pos->width = bit-diff;
                pos->count++;
                pos->child = {&node_t::nil, &node_t::nil, &node_t::nil, &node_t::nil};
                pos->child[b] = leaf;
                pos->child[nb] = inter;
                break;
            } else {
                pos->count++;
                bit -= pos->width;
                if(bit == 0) {
                    pos->sum = x;
                    break;
                }
                route[route_cnt++] = pos;
                int b = (v>>(bit-2))&3;
                if(pos->child[b] == &node_t::nil){
                    pos->child[b] = make_node(v, x, bit, 1);
                    break;
                }
                pos = pos->child[b];
            }
        }
        for(int i = route_cnt-1; i >= 0; i--){
            node_t *ptr = route[i];
            ptr->sum = op(op(op(ptr->child[0]->sum, ptr->child[1]->sum), ptr->child[2]->sum), ptr->child[3]->sum);
        }
    }
    
    int count(T v) const {
        node_t* pos = root;
        int bit = bit_width;
        while(pos != &node_t::nil){
            T mv = masked(v, bit-pos->width, bit);
            T mnv = masked(pos->value, bit-pos->width, bit);
            if(mv != mnv) return 0;
            bit -= pos->width;
            if(bit == 0) return pos->count;
            pos = pos->child[(v>>(bit-2))&3];
        }
        return 0;
    }
    
    int size() const {
        return root->count;
    }
    
    static node_t *meld(meldable_binary_trie &l, meldable_binary_trie &r){
        return meld(l.root, r.root);
    }
    static node_t *meld(node_t *l, node_t *r){
        node_t *res = meld(l, r, bit_width);
        return res;
    }
    static node_t *meld(node_t *l, node_t *r, int bit){
        if(l->count == 0){
            return r;
        }
        if(r->count == 0){
            return l;
        }
        T mlv = masked(l->value, min(bit-l->width, bit-r->width), bit);
        T mrv = masked(r->value, min(bit-l->width, bit-r->width), bit);
        if(mlv == mrv){
            if(l->width == r->width){
                l->count += r->count;
                if(bit == l->width){
                    l->sum = op(l->sum, r->sum);
                    return l;
                } else {
                    for(int i = 0; i < 4; i++) l->child[i] = meld(l->child[i], r->child[i], bit - l->width);
                    pool.push(r);
                }
            } else {
                if(l->width > r->width) swap(l, r);
                l->count += r->count;
                bit -= l->width;
                r->width -= l->width;
                int b = (r->value>>(bit-2))&3;
                l->child[b] = meld(l->child[b], r, bit);
            }
        } else {
            if(l->width > r->width) swap(l, r);
            int diff = diff_bit(mlv, mrv);
            if(l->width <= bit - diff){
                l->count += r->count;
                r->width -= l->width;
                bit -= l->width;
                int b = (r->value>>(bit-2))&3;
                l->child[b] = meld(l->child[b], r, bit);
            } else {
                node_t *ptr = make_node(l->value, l->sum, l->width-bit+diff, l->count, l->child[0], l->child[1], l->child[2], l->child[3]);
                memcpy(l->child.data(), node_t::nil.child.data(), sizeof(node_t*) * 4);
                r->width -= bit - diff;
                l->width = bit - diff;
                l->count += r->count;
                l->child[l->value>>(diff-2)&3] = ptr;
                l->child[r->value>>(diff-2)&3] = r;
            }
        }
        l->sum = op(op(op(l->child[0]->sum, l->child[1]->sum), l->child[2]->sum), l->child[3]->sum);
        return l;
    }
    
    pair<meldable_binary_trie, meldable_binary_trie> split(int p){
        if(rev) p = root->count-p;
        auto res = split(root, p);
        if(rev){
            meldable_binary_trie f(res.second), s(res.first);
            f.rev = true; s.rev = true;
            return make_pair(move(f), move(s));
        } else return {meldable_binary_trie(res.first), meldable_binary_trie(res.second)};
    }
    
    static pair<node_t*, node_t*> split(node_t *l, int p){
        if(p == 0) return {&node_t::nil, l};
        if(p == l->count) return {l, &node_t::nil};
        int acc = 0;
        
        for (int i = 0; i < 4; i++) {
            int c = l->child[i]->count;
            if (p < acc + c) {
                auto sub = split(l->child[i], p - acc);
                
                node_t *ptr = make_node(l->value, e(), l->width, 0);
                
                l->child[i] = sub.first;
                ptr->child[i] = sub.second;
                for(int j = i+1; j < 4; j++) ptr->child[j] = l->child[j];
                for(int j = i+1; j < 4; j++) l->child[j] = &node_t::nil;
                
                l->count = 0;
                for(int j = 0; j <= i; j++) l->count += l->child[j]->count;
                for(int j = i; j < 4; j++) ptr->count += ptr->child[j]->count;
                
                l->sum = e();
                for(int j = 0; j <= i; j++) l->sum = op(l->sum, l->child[j]->sum);
                for(int j = i; j < 4; j++) ptr->sum = op(ptr->sum, ptr->child[j]->sum);
                
                return {l, ptr};
            }
            acc += c;
        }
        assert(false);
    }
    
    meldable_binary_trie split_one(){
        if(rev){
            meldable_binary_trie res(split_one_rev(root));
            res.rev = true;
            return move(res);
        } else {
            return meldable_binary_trie(split_one(root));
        }
    }
    node_t *split_one(node_t *l){
        if(l->count == 1) return &node_t::nil;
        
        l->count--;
        for(int i = 0; i < 4; i++){
            int c = l->child[i]->count;
            if(c == 0) continue;
            l->child[i] = split_one(l->child[i]);
            l->sum = e();
            for(int j = i; j < 4; j++) l->sum = op(l->sum, l->child[j]->sum);
            return l;
        }
        assert(false);
    }
    node_t *split_one_rev(node_t *l){
        if(l->count == 1) return &node_t::nil;
        
        l->count--;
        for(int i = 3; i >= 0; i--){
            int c = l->child[i]->count;
            if(c == 0) continue;
            l->child[i] = split_one_rev(l->child[i]);
            l->sum = e();
            for(int j = 0; j <= i; j++) l->sum = op(l->sum, l->child[j]->sum);
            return l;
        }
        assert(false);
    }
    
    S all_prod() const {return root->sum;}
    
    void init(T p, const S &x){
        root = make_node(p, x, bit_width, 1);
    }
    
    static int get_size(node_t *ptr){
        if(ptr == &node_t::nil) return 0;
        int res = 1;
        for(int i = 0; i < 4; i++) res += get_size(ptr->child[i]);
        return res;
    }
};
template<typename T, typename S, auto op, auto e> typename meldable_binary_trie<T, S, op, e>::node_t meldable_binary_trie<T, S, op, e>::node_t::nil = meldable_binary_trie<T, S, op, e>::node_t(0, e(), bit_width, 0);
template<typename T, typename S, auto op, auto e> typename meldable_binary_trie<T, S, op, e>::Pool meldable_binary_trie<T, S, op, e>::pool = meldable_binary_trie<T, S, op, e>::Pool();
struct fastset{
    using ull = unsigned long long;
    using uint = unsigned;
    
    int siz;
    vector<ull> node;
    
    fastset(int _n){
        int n = 1;
        while((n<<6) < _n) n <<= 6;
        siz = (n+1)/63;
        node.resize((n+1)/63 + (_n+63)/64);
    }
    
    void insert(int x){
        uint idx = siz + (x/64);
        x &= 63;
        while(true){
            if((node[idx]>>x) & 1ull) return;
            node[idx] |= (1ull<<x);
            if(idx == 0) return;
            idx--;
            x = idx & 63;
            idx /= 64;
        }
    }
    
    void erase(int x){
        int idx = siz + (x/64);
        x &= 63;
        while(true){
            node[idx] &= ~(1ull<<x);
            if(idx == 0 || node[idx]) return;
            idx--;
            x = idx & 63;
            idx /= 64;
        }
    }
    
    bool count(int x) const {
        return (node[siz+x/64]>>(x&63u))&1;
    }
    
    int lower_bound(int x){
        if(count(x)) return x;
        int idx = siz + (x/64);
        x &= 63;
        while(true){
            if(node[idx] & ~(((1ull<<x)<<1) - 1ull)){
                x = __builtin_ctzll(node[idx] & ~(((1ull<<x)<<1) - 1ull));
                if(idx >= siz) return (idx-siz)*64+x;
                break;
            }
            if(idx == 0) return -1;
            idx--;
            x = idx & 63;
            idx /= 64;
        }
        
        idx = idx*64 + x+1;
        while(idx < siz) idx = idx*64 + __builtin_ctzll(node[idx])+1;
        return (idx-siz)*64 + __builtin_ctzll(node[idx]);
    }
    
    int less_bound(int x){
        if(count(x)) return x;
        int idx = siz + (x/64);
        x &= 63;
        while(true){
            if(node[idx] & ((1ull<<x) - 1ull)){
                x = 63 - __builtin_clzll(node[idx] & ((1ull<<x) - 1ull));
                if(idx >= siz) return (idx-siz)*64+x;
                break;
            }
            if(idx == 0) return -1;
            idx--;
            x = idx & 63;
            idx /= 64;
        }
        
        idx = idx*64 + x+1;
        while(idx < siz) idx = idx*64 + 64 - __builtin_clzll(node[idx]);
        return (idx-siz)*64 + 63 - __builtin_clzll(node[idx]);
    }
};
template<typename S, auto op, auto e>
struct segtree{
    int siz;
    vector<S> node;
    
    segtree() = default;
    segtree(unsigned n) : siz(std::bit_ceil(n)), node(siz*2, e()){}
    segtree(const vector<S>& v){init(v);}
    void init(const vector<S>& v){
        siz = bit_ceil(v.size()); node.resize(siz*2, e());
        for(int i = 0; i < (int)v.size(); i++) node[i+siz] = v[i];
        for(int i = siz-1; i >= 1; i--) node[i] = op(node[i*2], node[i*2+1]);
    }
    
    const S &operator[](int pos) const {return node[pos+siz];}
    
    const S &get(int pos) const {return node[pos+siz];}
    
    void set(int pos, const S &x){
        assert(0 <= pos && pos < siz);
        pos += siz;
        node[pos] = x;
        while(pos>>=1) node[pos] = op(node[pos<<1], node[(pos<<1)+1]);
    }
    
    void add(int pos, const S &x){
        assert(0 <= pos && pos < siz);
        pos += siz;
        node[pos] = op(node[pos], x);
        while(pos>>=1) node[pos] = op(node[pos], x);
    }
    
    S prod(int left, int right){
        left = max(0, left); right = min(right, siz);
        S l_ans = e(), r_ans = e();
        for(left+=siz, right+=siz; left < right; left>>=1, right>>=1){
            if(left&1) l_ans = op(l_ans, node[left++]);
            if(right&1) r_ans = op(node[--right], r_ans);
        }
        return op(l_ans, r_ans);
    }
    
    S all_prod(){
        return node[1];
    }
};

} //namespace smkyb

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
    smkyb::meldable_binary_trie<T, SS, SS::SS_op, SS::SS_e> *trie;
    smkyb::fastset fset;
    smkyb::segtree<S, op, e> seg;
    
    sort_segtree(int _n) : n(_n), fset(n+1), seg(n) {
        trie = new smkyb::meldable_binary_trie<T, SS, SS::SS_op, SS::SS_e>[n]{};
        for(int i = 0; i <= n; i++) fset.insert(i);
    }
    sort_segtree(const vector<pair<T, S>> &v) : n(v.size()), fset(n+1), seg(n) {
        trie = new smkyb::meldable_binary_trie<T, SS, SS::SS_op, SS::SS_e>[n];
        for(int i = 0; i <= n; i++) fset.insert(i);
        for(int i = 0; i < n; i++) trie[i] = smkyb::meldable_binary_trie<T, SS, SS::SS_op, SS::SS_e>(v[i].first, v[i].second);
        for(int i = 0; i < n; i++) seg.set(i, v[i].second);
    }
    sort_segtree(sort_segtree<T, S, op, e> &&o) = default;
    
    void set(int i, T k, S x) {
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
            auto res = smkyb::meldable_binary_trie<T, SS, SS::SS_op, SS::SS_e>::meld(trie[l], trie[lb]);
            trie[l] = smkyb::meldable_binary_trie<T, SS, SS::SS_op, SS::SS_e>(res);
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
            auto res = smkyb::meldable_binary_trie<T, SS, SS::SS_op, SS::SS_e>::meld(trie[l], trie[lb]);
            trie[l] = smkyb::meldable_binary_trie<T, SS, SS::SS_op, SS::SS_e>(res);
            fset.erase(lb);
            seg.set(lb, e());
            lb = fset.lower_bound(lb);
        }
        trie[l].rev = true;
        seg.set(l, trie[l].all_prod().rtol);
    }
};
