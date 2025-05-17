template <typename T>
struct binary_trie{
    private:
    
    struct node_t{
        T value;
        int count = 0;
        array<int, 4> child = {};
        int width = 0;
    };
    
    struct ref_node_t{
        T val;
        bool exist;
        ref_node_t(T x, bool e) : val{x}, exist{e} {}
    };
    
    static constexpr T one = 1;
    static constexpr int bit_width = sizeof(T) * 8;
    vector<node_t> node;
    T xor_val = 0;
    int root = -1;
    int siz = 0;
    
    //__builtin_clz
    template<typename _Tp>
    inline int clz(_Tp x) const {
        if constexpr(sizeof(_Tp) == 8ull) return __builtin_clzll(x);
        else return __builtin_clz(x);
    }
    
    inline T mask(int l, int r) const {
        if(r >= bit_width) return -(one<<l);
        return (one<<r) - (one<<l);
    }
    
    inline T masked(T v, int l, int r) const {
        return mask(l, r) & v;
    }
    
    inline int diff_bit(T x, T y) const {
        return ((bit_width-1 - clz(x^y))|1) + 1;
    }
    
    inline int make_node(T v){
        node.emplace_back();
        node.back().value = v;
        return ssize(node)-1;
    }
    
    public:
    
    binary_trie(){
        make_node(0);
        root = make_node(0);
        node[root].width = bit_width;
    }
    
    binary_trie<T>& operator=(binary_trie<T>&& o) noexcept = default;
    
    binary_trie(binary_trie&& o) noexcept = default;
    
    void insert(T v) {
        int pos = root;
        int bit = bit_width;
        siz++;
        v ^= xor_val;
        while(pos != 0){
            T mv = masked(v, bit-node[pos].width, bit);
            T mnv = masked(node[pos].value, bit-node[pos].width, bit);
            if(mv != mnv){
                int diff = diff_bit(mv, mnv);
                int b = (mv>>(diff-2))&3;
                int nb = (mnv>>(diff-2))&3;
                int inter = make_node(node[pos].value);
                int leaf = make_node(v);
                node[inter] = node[pos];
                node[inter].width -= bit - diff;
                memset(node[pos].child.data(), 0, sizeof(int)*4);
                node[pos].child[b] = leaf;
                node[pos].child[nb] = inter;
                node[pos].count++;
                node[pos].width = bit - diff;
                bit = diff;
                node[leaf].width = bit;
                node[leaf].count = 1;
                pos = leaf;
                return;
            } else {
                node[pos].count++;
                bit -= node[pos].width;
                if(bit == 0) return;
                int nex = node[pos].child[(v>>(bit-2))&3];
                if(nex == 0){
                    nex = node[pos].child[(v>>(bit-2))&3] = make_node(v);
                    node[nex].count = 1;
                    node[nex].width = bit;
                    return;
                }
                pos = nex;
            }
        }
    }
    
    int count(T v) const {
        int pos = root;
        int bit = bit_width;
        v ^= xor_val;
        while(pos != 0){
            T mv = masked(v, bit-node[pos].width, bit);
            T mnv = masked(node[pos].value, bit-node[pos].width, bit);
            if(mv != mnv) return 0;
            bit -= node[pos].width;
            if(bit == 0) return node[pos].count;
            pos = node[pos].child[(v>>(bit-2))&3];
        }
        return 0;
    }
    
    void erase(T v, int n = -1) {
        if(n == -1) n = count(v);
        if(n == 0) return;
        int pos = root;
        int bit = bit_width;
        siz -= n;
        v ^= xor_val;
        while(true){
            node[pos].count -= n;
            bit -= node[pos].width;
            if(bit == 0) return;
            pos = node[pos].child[(v>>(bit-2))&3];
        }
    }
    
    const ref_node_t operator[](int k) const {
        if(k >= 0) k = siz-k-1;
        else k += siz;
        if(k < 0 || siz <= k) return ref_node_t{0, false};
        
        k++;
        int pos = root;
        int bit = bit_width;
        while(true){
            bit -= node[pos].width;
            if(bit == 0) return ref_node_t{node[pos].value^xor_val, true};
            int b = (xor_val>>(bit-2))&3;
            auto &child = node[pos].child;
            if(k <= node[child[b^3]].count){
                pos = child[b^3];
            } else {
                k -= node[child[b^3]].count;
                if(k <= node[child[b^2]].count){
                    pos = child[b^2];
                } else {
                    k -= node[child[b^2]].count;
                    if(k <= node[child[b^1]].count){
                        pos = child[b^1];
                    } else {
                        k -= node[child[b^1]].count;
                        pos = child[b];
                    }
                }
            }
        }
    }
    
    int order(T v) const {
        if(v == 0) return 0;
        v--;
        int res = 0;
        int pos = root;
        int bit = bit_width;
        while(pos != 0){
            T mv = masked(v, bit-node[pos].width, bit);
            T mnv = masked(node[pos].value^xor_val, bit-node[pos].width, bit);
            if(mv < mnv){
                return res;
            } else {
                if(mv > mnv){
                    res += node[pos].count;
                    return res;
                } else {
                    bit -= node[pos].width;
                    if(bit == 0) return res + node[pos].count;
                    int b = (v>>(bit-2))&3;
                    auto &child = node[pos].child;
                    T mxv = (xor_val>>(bit-2))&3;
                    
                    if(b >= 1){
                        res += node[child[mxv]].count;
                        if(b >= 2){
                            res += node[child[mxv^1]].count;
                            if(b >= 3){
                                res += node[child[mxv^2]].count;
                            }
                        }
                    }
                    pos = node[pos].child[b^((xor_val>>(bit-2))&3)];
                }
            }
        }
        return res;
    }
    
    const ref_node_t lower_bound(T v) const {
        int ord = order(v);
        if(siz == ord) return ref_node_t{0, false};
        else return ref_node_t{(*this)[ord].val, true};
    }
    
    const ref_node_t less_bound(T v) const {
        int ord = v!=numeric_limits<T>::max() ? order(v+1) : siz;
        if(ord == 0) return ref_node_t{0, false};
        else return ref_node_t{(*this)[ord-1].val, true};
    }
    
    void reserve(int n) {
        node.reserve(2*n+2);
    }
    
    int size() const {
        return siz;
    }
    
    void apply_xor(T x) {
        xor_val ^= x;
    }
    
    const ref_node_t xor_min(T v) {
        if(siz == 0) return ref_node_t{0, false};
        apply_xor(v);
        ref_node_t res{(*this)[0].val^v, true};
        apply_xor(v);
        return res;
    }
    
    const ref_node_t xor_max(T v) {
        if(siz == 0) return ref_node_t{0, false};
        apply_xor(v);
        ref_node_t res{(*this)[-1].val^v, true};
        apply_xor(v);
        return res;
    }
};
