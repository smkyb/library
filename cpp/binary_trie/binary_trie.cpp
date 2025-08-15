template <typename T>
struct binary_trie{
    static_assert(is_unsigned_v<T>);
    
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
        ref_node_t(T x, bool e) : val{x}, exist{e} {}
    };
    
    struct Pool {
        constexpr static int SIZ = 1<<16;
        node_t* ptr = nullptr, *en = nullptr;
        node_t* get() {
            if(ptr == en) [[unlikely]] {
                ptr = new node_t[SIZ];
                en = ptr + SIZ;
            }
            return ptr++;
        }
    };
    
    static constexpr T one = 1;
    static constexpr int bit_width = sizeof(T) * 8;
    static Pool pool;
    T xor_val = 0;
    node_t* root;
    int siz = 0;
    
    template<typename _Tp>
    inline int clz(_Tp x) const {
        if constexpr(sizeof(_Tp) == 8ull) return __builtin_clzll(x);
        else return __builtin_clz(x);
    }
    
    inline T mask(int l, int r) const {
        if(r >= bit_width){
            if(l >= bit_width) return 0;
            else return -(one<<l);
        }
        return (one<<r) - (one<<l);
    }
    
    inline T masked(T v, int l, int r) const {
        return mask(l, r) & v;
    }
    
    inline int diff_bit(T x, T y) const {
        return ((bit_width-1 - clz(x^y))|1) + 1;
    }
    
    template<typename... Args>
    inline node_t* make_node(Args ...args) {
        return &((*pool.get()) = node_t(args...));
    }
    
    public:
    
    binary_trie(){
        root = make_node(0, bit_width, 0);
    }
    
    binary_trie<T>& operator=(binary_trie<T>&& o) noexcept = default;
    
    binary_trie(binary_trie&& o) noexcept = default;
    
    void insert(T v) {
        node_t* pos = root;
        int bit = bit_width;
        siz++;
        v ^= xor_val;
        while(true){
            T mv = masked(v, bit-pos->width, bit);
            T mnv = masked(pos->value, bit-pos->width, bit);
            if(mv != mnv){
                int diff = diff_bit(mv, mnv);
                int b = (mv>>(diff-2))&3;
                int nb = (mnv>>(diff-2))&3;
                node_t* inter = make_node(pos->value, pos->width-bit+diff, pos->count, pos->child[0], pos->child[1], pos->child[2], pos->child[3]);
                node_t* leaf = make_node(v, diff, 1);
                pos->width = bit-diff;
                pos->count++;
                pos->child = {&node_t::nil, &node_t::nil, &node_t::nil, &node_t::nil};
                pos->child[b] = leaf;
                pos->child[nb] = inter;
                return;
            } else {
                pos->count++;
                bit -= pos->width;
                if(bit == 0) return;
                int b = (v>>(bit-2))&3;
                if(pos->child[b] == &node_t::nil){
                    pos->child[b] = make_node(v, bit, 1);
                    return;
                }
                pos = pos->child[b];
            }
        }
    }
    
    int count(T v) const {
        node_t* pos = root;
        int bit = bit_width;
        v ^= xor_val;
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
    
    void erase(T v) {
        node_t* pos = root;
        int bit = bit_width;
        siz--;
        v ^= xor_val;
        while(true){
            pos->count--;
            bit -= pos->width;
            if(bit == 0) return;
            pos = pos->child[(v>>(bit-2))&3];
        }
    }
    
    void erase_all(T v) {
        int n = count(v);
        if(n == 0) return;
        node_t* pos = root;
        int bit = bit_width;
        siz -= n;
        v ^= xor_val;
        while(true){
            pos->count -= n;
            bit -= pos->width;
            if(bit == 0) return;
            pos = pos->child[(v>>(bit-2))&3];
        }
    }
    
    const T operator[](int k) const {
        if(k < 0) k += siz;
        
        k++;
        node_t* pos = root;
        int bit = bit_width;
        while(true){
            bit -= pos->width;
            if(bit == 0) return pos->value^xor_val;
            int b = (xor_val>>(bit-2))&3;
            auto &child = pos->child;
            if(k <= child[b]->count){
                pos = child[b];
            } else {
                k -= child[b]->count;
                if(k <= child[b^1]->count){
                    pos = child[b^1];
                } else {
                    k -= child[b^1]->count;
                    if(k <= child[b^2]->count){
                        pos = child[b^2];
                    } else {
                        k -= child[b^2]->count;
                        pos = child[b^3];
                    }
                }
            }
        }
    }
    
    T min() const {
        node_t* pos = root;
        int bit = bit_width;
        while(true){
            bit -= pos->width;
            if(bit == 0) return pos->value^xor_val;
            int b = (xor_val>>(bit-2))&3;
            auto &child = pos->child;
            if(child[b]->count) pos = child[b];
            else if(child[b^1]->count) pos = child[b^1];
            else if(child[b^2]->count) pos = child[b^2];
            else pos = child[b^3];
        }
    }
    
    T max() const {
        node_t* pos = root;
        int bit = bit_width;
        while(true){
            bit -= pos->width;
            if(bit == 0) return pos->value^xor_val;
            int b = (xor_val>>(bit-2))&3;
            auto &child = pos->child;
            if(child[b^3]->count) pos = child[b^3];
            else if(child[b^2]->count) pos = child[b^2];
            else if(child[b^1]->count) pos = child[b^1];
            else pos = child[b];
        }
    }
    
    int order(T v) const {
        if(v == 0) return 0;
        v--;
        int res = 0;
        node_t* pos = root;
        int bit = bit_width;
        while(pos != &node_t::nil){
            T mv = masked(v, bit-pos->width, bit);
            T mnv = masked(pos->value^xor_val, bit-pos->width, bit);
            if(mv < mnv){
                return res;
            } else {
                if(mv > mnv){
                    res += pos->count;
                    return res;
                } else {
                    bit -= pos->width;
                    if(bit == 0) return res + pos->count;
                    int b = (v>>(bit-2))&3;
                    auto &child = pos->child;
                    T mxv = (xor_val>>(bit-2))&3;
                    
                    if(b >= 1){
                        res += child[mxv]->count;
                        if(b >= 2){
                            res += child[mxv^1]->count;
                            if(b >= 3){
                                res += child[mxv^2]->count;
                            }
                        }
                    }
                    pos = child[b^mxv];
                }
            }
        }
        return res;
    }
    
    const ref_node_t lower_bound(T v) const {
        int ord = order(v);
        if(siz == ord) return ref_node_t{0, false};
        else return ref_node_t{(*this)[ord], true};
    }
    
    const ref_node_t less_bound(T v) const {
        int ord = v!=numeric_limits<T>::max() ? order(v+1) : siz;
        if(ord == 0) return ref_node_t{0, false};
        else return ref_node_t{(*this)[ord-1], true};
    }
    
    int size() const {
        return siz;
    }
    
    void apply_xor(T x) {
        xor_val ^= x;
    }
    
    const T xor_min(T v) {
        apply_xor(v);
        T res = min()^v;
        apply_xor(v);
        return res;
    }
    
    const T xor_max(T v) {
        apply_xor(v);
        T res = max()^v;
        apply_xor(v);
        return res;
    }
};
template<typename T> typename binary_trie<T>::node_t binary_trie<T>::node_t::nil = node_t(0, bit_width, 0, &node_t::nil, &node_t::nil, &node_t::nil, &node_t::nil);
template<typename T> typename binary_trie<T>::Pool binary_trie<T>::pool = Pool();

