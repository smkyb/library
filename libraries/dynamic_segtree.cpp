// 注意: 整数型 T (sizeof(T) <= 8、bool 不可)・C++17 以降 (GCC/Clang) 専用; ノード数上限 2^32-1
//       prod(l, r) は半開区間 [l, r) のため T の最大値は含められない (全体は all_prod() を使用)
//       reserve(n) は n >= 0 必須
template <typename T, typename S, auto op, auto e>
struct dynamic_segtree{
    static_assert(is_integral_v<T> && !is_same_v<T, bool> && sizeof(T) <= 8);
    using U = make_unsigned_t<T>;
    using ShiftU = conditional_t<sizeof(U) == 8, unsigned long long, unsigned int>;
    
    private:
    
    struct node_t{
        S sum;
        U value = 0;
        uint32_t child[2] = {0, 0};
        uint8_t width = 0;
        node_t() : sum{e()} {}
        node_t(U v, S x, uint8_t w, uint32_t c0 = 0, uint32_t c1 = 0)
            : sum(move(x)), value(v), child{c0, c1}, width(w) {}
    };
    
    static constexpr ShiftU one = 1;
    static constexpr int bit_width = sizeof(U) * 8;
    vector<node_t> node;
    uint32_t root = 0;
    
    static constexpr U to_u(T x) {
        if constexpr(is_signed_v<T>) return static_cast<U>(x) ^ static_cast<U>(one << (bit_width - 1));
        else return x;
    }
    
    template<typename _Tp>
    static inline int clz(_Tp x) {
        if constexpr(sizeof(_Tp) == 8ull) return __builtin_clzll(x);
        else return __builtin_clz(static_cast<unsigned int>(x)) - (32 - bit_width);
    }
    
    inline uint32_t make_node(U v, S x, uint8_t w, uint32_t c0 = 0, uint32_t c1 = 0){
        node.emplace_back(v, move(x), w, c0, c1);
        return static_cast<uint32_t>(node.size() - 1);
    }
    
    template<bool is_add>
    void _update(U v, const S &x){
        if(root == 0){
            root = make_node(v, x, static_cast<uint8_t>(bit_width));
            return;
        }
        uint32_t route[65];
        int rcnt = 0;
        uint32_t pos = root;
        int bit = bit_width;
        while(true){
            int w = node[pos].width;
            int nbit = bit - w;
            U x_xor = (w == 0) ? U(0) : static_cast<U>((v ^ node[pos].value) >> nbit);
            if(x_xor != 0){
                int d = bit_width - clz(x_xor) + nbit;
                U pv = node[pos].value;
                S ps = move(node[pos].sum);
                uint32_t c0 = node[pos].child[0], c1 = node[pos].child[1];
                uint8_t iw = static_cast<uint8_t>(w - (bit - d));
                uint32_t inter = make_node(pv, move(ps), iw, c0, c1);
                uint32_t leaf = make_node(v, x, static_cast<uint8_t>(d));
                int b = static_cast<int>((v >> (d - 1)) & 1);
                node[pos].width = static_cast<uint8_t>(bit - d);
                node[pos].child[b] = leaf;
                node[pos].child[b ^ 1] = inter;
                route[rcnt++] = pos;
                break;
            }
            bit = nbit;
            if(bit == 0){
                if constexpr (is_add) node[pos].sum = op(node[pos].sum, x);
                else node[pos].sum = x;
                break;
            }
            route[rcnt++] = pos;
            pos = node[pos].child[(v >> (bit - 1)) & 1];
        }
        for(int i = rcnt - 1; i >= 0; --i){
            uint32_t idx = route[i];
            node[idx].sum = op(node[node[idx].child[0]].sum, node[node[idx].child[1]].sum);
        }
    }
    
    S _prod(U l, U r, uint32_t pos, int bit) const {
        int nbit = bit - node[pos].width;
        U mv, hi;
        if(nbit == bit_width){
            mv = 0;
            hi = static_cast<U>(~U(0));
        } else {
            U lm = (nbit == 0) ? U(0) : static_cast<U>((one << nbit) - 1);
            mv = node[pos].value & ~lm;
            hi = node[pos].value | lm;
        }
        if(r <= mv || hi < l) return e();
        if(l <= mv && hi < r) return node[pos].sum;
        U mid = static_cast<U>(mv | (one << (nbit - 1)));
        if(r <= mid) return _prod(l, r, node[pos].child[0], nbit);
        if(l >= mid) return _prod(l, r, node[pos].child[1], nbit);
        return op(_prod(l, r, node[pos].child[0], nbit), _prod(l, r, node[pos].child[1], nbit));
    }
    
    public:
    
    dynamic_segtree() : node(1) {}
    
    dynamic_segtree(const dynamic_segtree&) = default;
    dynamic_segtree& operator=(const dynamic_segtree&) = default;
    dynamic_segtree(dynamic_segtree&&) = default;
    dynamic_segtree& operator=(dynamic_segtree&&) = default;
    
    void set(T p, const S &x) { _update<false>(to_u(p), x); }
    
    void add(T p, const S &x) { _update<true>(to_u(p), x); }
    
    S get(T p) const {
        if(root == 0) return e();
        U up = to_u(p);
        uint32_t pos = root;
        int bit = bit_width;
        while(true){
            bit -= node[pos].width;
            if(bit == 0) return node[pos].value == up ? node[pos].sum : e();
            pos = node[pos].child[(up >> (bit - 1)) & 1];
        }
    }
    
    S prod(T l, T r) const {
        if(l >= r || root == 0) return e();
        return _prod(to_u(l), to_u(r), root, bit_width);
    }
    
    S all_prod() const {
        return node[root].sum;
    }
    
    void reserve(int n) {
        node.reserve(static_cast<size_t>(n) * 2 + 2);
    }
};