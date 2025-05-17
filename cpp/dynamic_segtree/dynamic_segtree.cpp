template <typename T, typename S, auto op, auto e>
struct dynamic_segtree{
    private:
    
    struct node_t{
        S sum;
        T value;
        array<int, 4> child = {};
        int width = 0;
        node_t() : sum{e()} {}
    };
    
    static constexpr T one = 1;
    static constexpr int bit_width = sizeof(T) * 8;
    vector<node_t> node;
    int root = -1;
    
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
    
    inline int make_node(T v, const S &x){
        node.emplace_back();
        node.back().value = v;
        node.back().sum = x;
        return ssize(node)-1;
    }
    
    public:
    
    binary_trie(){
        make_node(0, e());
        root = make_node(0, e());
        node[root].width = bit_width;
    }
    
    binary_trie<T, S, op, e>& operator=(binary_trie<T, S, op, e>&& o) noexcept = default;
    
    binary_trie(binary_trie&& o) noexcept = default;
    
    void set(T v, const S &x) {
        int pos = root;
        int bit = bit_width;
        int route[65], route_cnt = 0;
        while(true){
            T mv = masked(v, bit-node[pos].width, bit);
            T mnv = masked(node[pos].value, bit-node[pos].width, bit);
            if(mv != mnv){
                route[route_cnt++] = pos;
                int diff = diff_bit(mv, mnv);
                int b = (mv>>(diff-2))&3;
                int nb = (mnv>>(diff-2))&3;
                int inter = make_node(node[pos].value, node[pos].sum);
                int leaf = make_node(v, x);
                node[inter] = node[pos];
                node[inter].width -= bit - diff;
                memset(node[pos].child.data(), 0, sizeof(int)*4);
                node[pos].child[b] = leaf;
                node[pos].child[nb] = inter;
                node[pos].width = bit - diff;
                bit = diff;
                node[leaf].width = bit;
                pos = leaf;
                break;
            } else {
                bit -= node[pos].width;
                if(bit == 0){
                    node[pos].sum = x;
                    break;
                }
                route[route_cnt++] = pos;
                int nex = node[pos].child[(v>>(bit-2))&3];
                if(nex == 0){
                    nex = node[pos].child[(v>>(bit-2))&3] = make_node(v, x);
                    node[nex].width = bit;
                    break;
                }
                pos = nex;
            }
        }
        for(int i = route_cnt-1; i >= 0; i--){
            int idx = route[i];
            auto &child = node[idx].child;
            node[idx].sum = op(op(op(node[child[0]].sum, node[child[1]].sum), node[child[2]].sum), node[child[3]].sum);
        }
    }
    
    S prod(T l, T r) const {
        return _prod(l, r, root, bit_width);
    }
    S _prod(T l, T r, int pos, int bit) const {
        if(pos == 0) return e();
        bit -= node[pos].width;
        T mv = masked(node[pos].value, bit, bit_width);
        if(r <= mv) return e();
        if(l != numeric_limits<T>::min() && mv-1+(one<<bit) < l) return e();
        if(l <= mv && mv-1+(one<<bit) < r){
            return node[pos].sum;
        }
        auto &child = node[pos].child;
        return op(op(op(_prod(l, r, child[0], bit), _prod(l, r, child[1], bit)), _prod(l, r, child[2], bit)), _prod(l, r, child[3], bit));
    }
    
    void reserve(int n) {
        node.reserve(2*n+2);
    }
};
