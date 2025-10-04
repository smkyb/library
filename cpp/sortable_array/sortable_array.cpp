template<typename T>
struct sortable_array {
    struct fastset{
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
    
    struct binary_trie{
        static_assert(is_unsigned_v<T>);
        
        static constexpr T one = 1;
        static constexpr int bit_width = sizeof(T) * 8;
        
        struct node_t{
            T value;
            int width;
            int count;
            array<node_t*, 4> child;
            node_t() = default;
            node_t(T v, int w, int c) : value(v), width(w), count(c), child{&nil, &nil, &nil, &nil} {}
            node_t(T v, int w, int c, node_t* c0, node_t* c1, node_t* c2, node_t* c3) : value(v), width(w), count(c), child{c0, c1, c2, c3} {}
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
        
        binary_trie(){
            root = make_node(0, bit_width, 0);
        }
        binary_trie(T p){
            root = make_node(p, bit_width, 1);
        }
        binary_trie(node_t *x){
            root = x;
        }
        
        binary_trie& operator=(binary_trie&& o) noexcept = default;
        
        binary_trie(binary_trie&& o) noexcept = default;
        
        const T operator[](int k) const {
            k++;
            node_t* pos = root;
            int bit = bit_width;
            while(true){
                bit -= pos->width;
                if(bit == 0) return pos->value;
                auto &child = pos->child;
                if(k <= child[0]->count){
                    pos = child[0];
                } else {
                    k -= child[0]->count;
                    if(k <= child[1]->count){
                        pos = child[1];
                    } else {
                        k -= child[1]->count;
                        if(k <= child[2]->count){
                            pos = child[2];
                        } else {
                            k -= child[2]->count;
                            pos = child[3];
                        }
                    }
                }
            }
        }
        
        int size() const {
            return root->count;
        }
        
        static node_t *meld(binary_trie &l, binary_trie &r){
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
                    node_t *ptr = make_node(l->value, l->width-bit+diff, l->count, l->child[0], l->child[1], l->child[2], l->child[3]);
                    memcpy(l->child.data(), node_t::nil.child.data(), sizeof(node_t*) * 4);
                    r->width -= bit - diff;
                    l->width = bit - diff;
                    l->count += r->count;
                    l->child[l->value>>(diff-2)&3] = ptr;
                    l->child[r->value>>(diff-2)&3] = r;
                }
            }
            return l;
        }
        
        pair<binary_trie, binary_trie> split(int p){
            if(rev) p = root->count-p;
            auto res = split(root, p);
            if(rev){
                binary_trie f(res.second), s(res.first);
                f.rev = true; s.rev = true;
                return make_pair(move(f), move(s));
            } else return {binary_trie(res.first), binary_trie(res.second)};
        }
        
        static pair<node_t*, node_t*> split(node_t *l, int p){
            if(p == 0) return {&node_t::nil, l};
            if(p == l->count) return {l, &node_t::nil};
            int acc = 0;
            
            for (int i = 0; i < 4; i++) {
                int c = l->child[i]->count;
                if (p < acc + c) {
                    auto sub = split(l->child[i], p - acc);
                    
                    node_t *ptr = make_node(l->value, l->width, 0);
                    
                    l->child[i] = sub.first;
                    ptr->child[i] = sub.second;
                    for(int j = i+1; j < 4; j++) ptr->child[j] = l->child[j];
                    for(int j = i+1; j < 4; j++) l->child[j] = &node_t::nil;
                    
                    l->count = 0;
                    for(int j = 0; j <= i; j++) l->count += l->child[j]->count;
                    for(int j = i; j < 4; j++) ptr->count += ptr->child[j]->count;
                    
                    return {l, ptr};
                }
                acc += c;
            }
            assert(false);
        }
        
        binary_trie split_one(){
            if(rev){
                binary_trie res(split_one_rev(root));
                res.rev = true;
                return move(res);
            } else {
                return binary_trie(split_one(root));
            }
        }
        node_t *split_one(node_t *l){
            if(l->count == 1) return &node_t::nil;
            
            l->count--;
            for(int i = 0; i < 4; i++){
                int c = l->child[i]->count;
                if(c == 0) continue;
                l->child[i] = split_one(l->child[i]);
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
                return l;
            }
            assert(false);
        }
        
        void init(T p){
            root = make_node(p, bit_width, 1);
        }
        
        static int get_size(node_t *ptr){
            if(ptr == &node_t::nil) return 0;
            int res = 1;
            for(int i = 0; i < 4; i++) res += get_size(ptr->child[i]);
            return res;
        }
    };
    
    int n;
    binary_trie *trie;
    fastset fset;
    
    sortable_array(int _n) : n(_n), fset(n+1) {
        trie = new binary_trie[n]{};
        for(int i = 0; i <= n; i++) fset.insert(i);
    }
    sortable_array(const vector<T> &v) : n(v.size()), fset(n+1) {
        trie = new binary_trie[n];
        for(int i = 0; i <= n; i++) fset.insert(i);
        for(int i = 0; i < n; i++) trie[i] = binary_trie(v[i]);
    }
    sortable_array(sortable_array<T> &&o) = default;
    
    void set(int i, T k){
        int l = fset.less_bound(i);
        int r = fset.lower_bound(i+1);
        if(l < i){
            auto res = trie[l].split(i-l);
            trie[l] = move(res.first); trie[i] = move(res.second);
            fset.insert(i);
        }
        if(i+1 < r){
            auto res = trie[i].split_one();
            trie[i+1] = move(res);
            fset.insert(i+1);
        }
        trie[i].init(k);
    }
    
    T get(int i){
        int p = fset.less_bound(i);
        i -= p;
        return trie[p].rev ? trie[p][trie[p].size()-i-1] : trie[p][i];
    }
    
    void sort(int l, int r) {
        int lb = fset.less_bound(l);
        if(lb < l){
            auto res = trie[lb].split(l-lb);
            trie[lb] = move(res.first); trie[l] = move(res.second);
            fset.insert(l);
        }
        int rb = fset.less_bound(r);
        if(rb < r){
            auto res = trie[rb].split(r-rb);
            trie[rb] = move(res.first); trie[r] = move(res.second);
            fset.insert(r);
        }
        lb = fset.lower_bound(l+1);
        while(lb < r){
            auto res = binary_trie::meld(trie[l], trie[lb]);
            trie[l] = binary_trie(res);
            fset.erase(lb);
            lb = fset.lower_bound(lb);
        }
        trie[l].rev = false;
    }
    
    void sort_rev(int l, int r) {
        int lb = fset.less_bound(l);
        if(lb < l){
            auto res = trie[lb].split(l-lb);
            trie[lb] = move(res.first); trie[l] = move(res.second);
            fset.insert(l);
        }
        int rb = fset.less_bound(r);
        if(rb < r){
            auto res = trie[rb].split(r-rb);
            trie[rb] = move(res.first); trie[r] = move(res.second);
            fset.insert(r);
        }
        lb = fset.lower_bound(l+1);
        while(lb < r){
            auto res = binary_trie::meld(trie[l], trie[lb]);
            trie[l] = binary_trie(res);
            fset.erase(lb);
            lb = fset.lower_bound(lb);
        }
        trie[l].rev = true;
    }
};
template<typename T> typename sortable_array<T>::binary_trie::node_t sortable_array<T>::binary_trie::node_t::nil = node_t(0, bit_width, 0, &node_t::nil, &node_t::nil, &node_t::nil, &node_t::nil);
template<typename T> typename sortable_array<T>::binary_trie::Pool sortable_array<T>::binary_trie::pool = Pool();
