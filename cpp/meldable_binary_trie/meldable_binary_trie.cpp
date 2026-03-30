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
        array<node_t*, 2> child;
        node_t() = default;
        node_t(T v, S s, int w, int c) : value(v), sum(s), width(w), count(c), child{&nil, &nil} {}
        node_t(T v, S s, int w, int c, node_t* c0, node_t* c1) : value(v), sum(s), width(w), count(c), child{c0, c1} {}
        static node_t nil;
    };
    
    struct ref_node_t{
        T val;
        bool exist;
        ref_node_t(T x, bool _e) : val{x}, exist{_e} {}
    };
    
    struct Pool {
        constexpr static int SIZ = 1<<17;
        node_t *ptr = nullptr, *en = nullptr;
        vector<node_t*> reuse;
        Pool() {
            ptr = new node_t[SIZ];
            en = ptr + SIZ;
        }
        node_t *get() {
            if(!reuse.empty()) {
                auto ptr = reuse.back();
                reuse.pop_back();
                return ptr;
            }
            if(ptr == en) {
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
        if(r >= bit_width){
            if(l >= bit_width) return 0;
            else return -(one<<l);
        }
        return (one<<r) - (one<<l);
    }
    
    inline static T masked(T v, int l, int r) {
        return mask(l, r) & v;
    }
    
    inline static int diff_bit(T x, T y) {
        return bit_width - clz(x^y);
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
    
    int size() const {
        return root->count;
    }
    
    static node_t *meld(meldable_binary_trie &l, meldable_binary_trie &r){
        return meld(l.root, r.root, bit_width);
    }
    static node_t *meld(node_t *l, node_t *r){
        return meld(l, r, bit_width);
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
                l->child[0] = meld(l->child[0], r->child[0], bit - l->width);
                l->child[1] = meld(l->child[1], r->child[1], bit - l->width);
                pool.push(r);
            } else {
                if(l->width > r->width) swap(l, r);
                l->count += r->count;
                bit -= l->width;
                r->width -= l->width;
                bool b = (r->value>>(bit-1))&1;
                l->child[b] = meld(l->child[b], r, bit);
            }
        } else {
            if(l->width > r->width) swap(l, r);
            int diff = diff_bit(mlv, mrv);
            if(l->width <= bit - diff){
                l->count += r->count;
                r->width -= l->width;
                bit -= l->width;
                bool b = (r->value>>(bit-1))&1;
                l->child[b] = meld(l->child[b], r, bit);
            } else {
                node_t *ptr = make_node(l->value, l->sum, l->width-bit+diff, l->count, l->child[0], l->child[1]);
                r->width -= bit - diff;
                l->width = bit - diff;
                l->count += r->count;
                l->child[ptr->value>>(diff-1)&1] = ptr;
                l->child[r->value>>(diff-1)&1] = r;
            }
        }
        l->sum = op(l->child[0]->sum, l->child[1]->sum);
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
        
        if(p == l->child[0]->count) {
            l->child[0]->width += l->width;
            l->child[1]->width += l->width;
            pool.push(l);
            return {l->child[0], l->child[1]};
        } else if(p < l->child[0]->count) {
            auto [subl, subr] = split(l->child[0], p);
            
            subl->width += l->width;
            l->child[0] = subr;
            l->count = l->child[0]->count + l->child[1]->count;
            l->sum = op(l->child[0]->sum, l->child[1]->sum);
            return {subl, l};
        } else {
            auto [subl, subr] = split(l->child[1], p - l->child[0]->count);
            
            subr->width += l->width;
            l->child[1] = subl;
            l->count = l->child[0]->count + l->child[1]->count;
            l->sum = op(l->child[0]->sum, l->child[1]->sum);
            return {l, subr};
        }
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
        if(l->count == 1){
            pool.push(l);
            return &node_t::nil;
        }
        
        l->count--;
        if(l->child[0]->count > 0){
            l->child[0] = split_one(l->child[0]);
            l->sum = op(l->child[0]->sum, l->child[1]->sum);
        } else {
            l->child[1] = split_one(l->child[1]);
            l->sum = l->child[1]->sum;
        }
        return l;
    }
    node_t *split_one_rev(node_t *l){
        if(l->count == 1){
            pool.push(l);
            return &node_t::nil;
        }
        
        l->count--;
        if(l->child[1]->count > 0){
            l->child[1] = split_one_rev(l->child[1]);
            l->sum = op(l->child[0]->sum, l->child[1]->sum);
        } else {
            l->child[0] = split_one_rev(l->child[0]);
            l->sum = l->child[0]->sum;
        }
        return l;
    }
    
    S prod_l(int n) {
        return _prod_l(n, root);
    }
    
    S _prod_l(int n, node_t *pos) {
        if(n == 0) return e();
        if(n == pos->count) return pos->sum;
        
        if(n < pos->child[0]->count) return _prod_l(n, pos->child[0]);
        else if(n == pos->child[0]->count) return pos->child[0]->sum;
        else return op(pos->child[0]->sum, _prod_l(n - pos->child[0]->count, pos->child[1]));
    }
    
    S prod_r(int n) {
        return _prod_r(n, root);
    }
    
    S _prod_r(int n, node_t *pos) {
        if(n == 0) return e();
        if(n == pos->count) return pos->sum;
        
        if(n < pos->child[1]->count) return _prod_r(n, pos->child[1]);
        else if(n == pos->child[1]->count) return pos->child[1]->sum;
        else return op(_prod_r(n - pos->child[1]->count, pos->child[0]), pos->child[1]->sum);
    }
    
    S prod_lr(int l, int r) {
        return _prod_lr(l, r, root);
    }
    
    S _prod_lr(int l, int r, node_t *pos) {
        if(l == r) return e();
        if(l == 0) return _prod_l(r, pos);
        if(r == pos->count) return _prod_r(r-l, pos);
        
        const auto &child = pos->child;
        
        if(l == child[0]->count) return _prod_l(r-l, child[1]);
        if(r == child[0]->count) return _prod_r(r-l, child[0]);
        
        if(l < child[0]->count){
            if(r < child[0]->count) return _prod_lr(l, r, child[0]);
            else return op(_prod_r(child[0]->count-l, child[0]), _prod_l(r-child[0]->count, child[1]));
        } else {
            return _prod_lr(l-child[0]->count, r-child[0]->count, child[1]);
        }
    }
    
    S all_prod() const {return root->sum;}
    
    void init(T p, const S &x){
        root = make_node(p, x, bit_width, 1);
    }
    
    static int get_size(node_t *ptr){
        if(ptr == &node_t::nil) return 0;
        int res = 1;
        for(int i = 0; i < 2; i++) res += get_size(ptr->child[i]);
        return res;
    }
};
template<typename T, typename S, auto op, auto e> typename meldable_binary_trie<T, S, op, e>::node_t meldable_binary_trie<T, S, op, e>::node_t::nil = meldable_binary_trie<T, S, op, e>::node_t(0, e(), bit_width, 0);
template<typename T, typename S, auto op, auto e> typename meldable_binary_trie<T, S, op, e>::Pool meldable_binary_trie<T, S, op, e>::pool = meldable_binary_trie<T, S, op, e>::Pool();
