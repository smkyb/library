// 注意: 符号なし整数型 T (sizeof(T) <= 8) 専用・多重集合 (erase は 1 個削除 / 全削除は erase_all)
//       xor_min(x) / xor_max(x) は v^x の値ではなく最小/最大にする要素 v 自体を返す
//       max_leq(x) は x 以下 (x を含む) の最大値; lower_bound/max_leq は *r, .value_or(def), if(r) 対応
template <typename T>
struct binary_trie{
    static_assert(is_unsigned_v<T> && sizeof(T) <= 8);
    
    struct alignas(32) node_t{
        T value = 0;
        uint32_t child[4] = {0, 0, 0, 0};
        int count = 0;
        uint8_t width = 0;
        node_t() = default;
        node_t(T v, uint8_t w, int c, uint32_t c0 = 0, uint32_t c1 = 0, uint32_t c2 = 0, uint32_t c3 = 0)
            : value(v), child{c0, c1, c2, c3}, count(c), width(w) {}
    };
    
    struct ref_node_t{
        T val;
        bool exist;
        explicit operator bool() const { return exist; }
        T operator*() const { assert(exist); return val; }
        template<typename U = T>
        U value_or(U def) const { return exist ? static_cast<U>(val) : def; }
    };
    
    static constexpr int bit_width = sizeof(T) * 8;
    vector<node_t> nodes;
    uint32_t root = 0;
    uint32_t free_head = 0;
    T xor_val = 0;
    int siz = 0;
    
    template<typename _Tp>
    static inline int clz(_Tp x) {
        if constexpr(sizeof(_Tp) == 8ull) return __builtin_clzll(x);
        else return __builtin_clz(static_cast<unsigned int>(x)) - (32 - bit_width);
    }
    
    inline uint32_t make_node(T v, uint8_t w, int c, uint32_t c0 = 0, uint32_t c1 = 0, uint32_t c2 = 0, uint32_t c3 = 0){
        if(__builtin_expect(free_head != 0, 0)){
            uint32_t id = free_head;
            free_head = nodes[id].child[0];
            nodes[id] = node_t(v, w, c, c0, c1, c2, c3);
            return id;
        }
        nodes.emplace_back(v, w, c, c0, c1, c2, c3);
        return static_cast<uint32_t>(nodes.size() - 1);
    }
    
    inline void push_free(uint32_t id){
        nodes[id].child[0] = free_head;
        free_head = id;
    }
    
    T _min_from(uint32_t pos, int bit, T xv) const {
        while(true){
            bit -= nodes[pos].width;
            if(bit == 0) return nodes[pos].value ^ xor_val;
            int xb = static_cast<int>((xv >> (bit - 2)) & 3);
            uint32_t nxt = nodes[pos].child[xb];
            if(nxt == 0) nxt = nodes[pos].child[xb ^ 1];
            if(nxt == 0) nxt = nodes[pos].child[xb ^ 2];
            if(nxt == 0) nxt = nodes[pos].child[xb ^ 3];
            pos = nxt;
        }
    }
    
    T _max_from(uint32_t pos, int bit, T xv) const {
        while(true){
            bit -= nodes[pos].width;
            if(bit == 0) return nodes[pos].value ^ xor_val;
            int xb = static_cast<int>((xv >> (bit - 2)) & 3);
            uint32_t nxt = nodes[pos].child[xb ^ 3];
            if(nxt == 0) nxt = nodes[pos].child[xb ^ 2];
            if(nxt == 0) nxt = nodes[pos].child[xb ^ 1];
            if(nxt == 0) nxt = nodes[pos].child[xb];
            pos = nxt;
        }
    }
    
    void erase_impl(T v, bool all_flag){
        if(root == 0) return;
        T key = v ^ xor_val;
        uint32_t path[33];
        uint8_t dir[33];
        int depth = 0;
        uint32_t pos = root;
        int bit = bit_width;
        while(true){
            int w = nodes[pos].width;
            int nbit = bit - w;
            T x_xor = (w == 0) ? T(0) : static_cast<T>((key ^ nodes[pos].value) >> nbit);
            if(x_xor != 0) return;
            bit = nbit;
            if(bit == 0) break;
            int b = static_cast<int>((key >> (bit - 2)) & 3);
            uint32_t nxt = nodes[pos].child[b];
            if(nxt == 0) return;
            path[depth] = pos;
            dir[depth] = static_cast<uint8_t>(b);
            ++depth;
            pos = nxt;
        }
        int del = all_flag ? nodes[pos].count : 1;
        siz -= del;
        nodes[pos].count -= del;
        if(nodes[pos].count == 0){
            push_free(pos);
            if(depth == 0){
                root = 0;
                return;
            }
            uint32_t p = path[depth - 1];
            nodes[p].child[dir[depth - 1]] = 0;
            nodes[p].count -= del;
            uint32_t sib = 0;
            int rem = 0;
            for(int j = 0; j < 4; ++j){
                if(nodes[p].child[j] != 0){
                    sib = nodes[p].child[j];
                    ++rem;
                }
            }
            if(rem == 1){
                nodes[sib].width = static_cast<uint8_t>(nodes[sib].width + nodes[p].width);
                push_free(p);
                if(depth == 1){
                    root = sib;
                    return;
                }
                nodes[path[depth - 2]].child[dir[depth - 2]] = sib;
            }
            for(int i = 0; i <= depth - 2; ++i) nodes[path[i]].count -= del;
        } else {
            for(int i = 0; i < depth; ++i) nodes[path[i]].count -= del;
        }
    }
    
    public:
    
    binary_trie() : nodes(1) {}
    binary_trie(const binary_trie&) = default;
    binary_trie& operator=(const binary_trie&) = default;
    binary_trie(binary_trie&&) noexcept = default;
    binary_trie& operator=(binary_trie&&) noexcept = default;
    
    void reserve(int n){
        nodes.reserve(static_cast<size_t>(n) * 2 + 2);
    }
    
    void insert(T x) {
        siz++;
        x ^= xor_val;
        if(root == 0){
            root = make_node(x, static_cast<uint8_t>(bit_width), 1);
            return;
        }
        uint32_t pos = root;
        int bit = bit_width;
        while(true){
            int w = nodes[pos].width;
            int nbit = bit - w;
            T x_xor = (w == 0) ? T(0) : static_cast<T>((x ^ nodes[pos].value) >> nbit);
            if(x_xor != 0){
                int d = (bit_width - clz(x_xor) + nbit + 1) & ~1;
                T pv = nodes[pos].value;
                int pc = nodes[pos].count;
                uint32_t c0 = nodes[pos].child[0], c1 = nodes[pos].child[1];
                uint32_t c2 = nodes[pos].child[2], c3 = nodes[pos].child[3];
                uint8_t iw = static_cast<uint8_t>(w - (bit - d));
                uint32_t inter = make_node(pv, iw, pc, c0, c1, c2, c3);
                uint32_t leaf = make_node(x, static_cast<uint8_t>(d), 1);
                int b_new = static_cast<int>((x >> (d - 2)) & 3);
                int b_old = static_cast<int>((pv >> (d - 2)) & 3);
                nodes[pos].width = static_cast<uint8_t>(bit - d);
                nodes[pos].count = pc + 1;
                nodes[pos].child[0] = nodes[pos].child[1] = nodes[pos].child[2] = nodes[pos].child[3] = 0;
                nodes[pos].child[b_new] = leaf;
                nodes[pos].child[b_old] = inter;
                return;
            }
            nodes[pos].count++;
            bit = nbit;
            if(bit == 0) return;
            int b = static_cast<int>((x >> (bit - 2)) & 3);
            uint32_t nxt = nodes[pos].child[b];
            if(nxt == 0){
                uint32_t leaf = make_node(x, static_cast<uint8_t>(bit), 1);
                nodes[pos].child[b] = leaf;
                return;
            }
            pos = nxt;
        }
    }
    
    int count(T x) const {
        if(root == 0) return 0;
        T key = x ^ xor_val;
        uint32_t pos = root;
        int bit = bit_width;
        while(true){
            int w = nodes[pos].width;
            int nbit = bit - w;
            T x_xor = (w == 0) ? T(0) : static_cast<T>((key ^ nodes[pos].value) >> nbit);
            if(x_xor != 0) return 0;
            bit = nbit;
            if(bit == 0) return nodes[pos].count;
            pos = nodes[pos].child[(key >> (bit - 2)) & 3];
            if(pos == 0) return 0;
        }
    }
    
    void erase(T x) { erase_impl(x, false); }
    
    void erase_all(T x) { erase_impl(x, true); }
    
    const T operator[](int k) const {
        if(k < 0) k += siz;
        assert(0 <= k && k < siz);
        k++;
        uint32_t pos = root;
        int bit = bit_width;
        while(true){
            bit -= nodes[pos].width;
            if(bit == 0) return nodes[pos].value ^ xor_val;
            int xb = static_cast<int>((xor_val >> (bit - 2)) & 3);
            for(int i = 0; i < 4; ++i){
                uint32_t c = nodes[pos].child[i ^ xb];
                if(c == 0) continue;
                int cnt = nodes[c].count;
                if(k <= cnt){
                    pos = c;
                    break;
                }
                k -= cnt;
            }
        }
    }
    
    T min() const {
        assert(siz > 0);
        return _min_from(root, bit_width, xor_val);
    }
    
    T max() const {
        assert(siz > 0);
        return _max_from(root, bit_width, xor_val);
    }
    
    int count_less(T x) const {
        if(root == 0) return 0;
        int res = 0;
        uint32_t pos = root;
        int bit = bit_width;
        while(true){
            int w = nodes[pos].width;
            int nbit = bit - w;
            T pv_eff = nodes[pos].value ^ xor_val;
            T x_xor = (w == 0) ? T(0) : static_cast<T>((pv_eff ^ x) >> nbit);
            if(x_xor != 0){
                int d = bit_width - clz(x_xor) + nbit;
                if(((pv_eff >> (d - 1)) & 1) == 0) res += nodes[pos].count;
                return res;
            }
            bit = nbit;
            if(bit == 0) return res;
            int xb = static_cast<int>((xor_val >> (bit - 2)) & 3);
            int vb = static_cast<int>((x >> (bit - 2)) & 3);
            for(int i = 0; i < vb; ++i){
                uint32_t c = nodes[pos].child[i ^ xb];
                if(c != 0) res += nodes[c].count;
            }
            pos = nodes[pos].child[vb ^ xb];
            if(pos == 0) return res;
        }
    }
    
    ref_node_t lower_bound(T x) const {
        if(root == 0) return ref_node_t{0, false};
        uint32_t cand = 0;
        int cand_bit = 0;
        uint32_t pos = root;
        int bit = bit_width;
        while(true){
            int w = nodes[pos].width;
            int nbit = bit - w;
            T pv_eff = nodes[pos].value ^ xor_val;
            T x_xor = (w == 0) ? T(0) : static_cast<T>((pv_eff ^ x) >> nbit);
            if(x_xor != 0){
                int d = bit_width - clz(x_xor) + nbit;
                if((pv_eff >> (d - 1)) & 1) return ref_node_t{_min_from(pos, bit, xor_val), true};
                return cand ? ref_node_t{_min_from(cand, cand_bit, xor_val), true} : ref_node_t{0, false};
            }
            bit = nbit;
            if(bit == 0) return ref_node_t{x, true};
            int xb = static_cast<int>((xor_val >> (bit - 2)) & 3);
            int vb = static_cast<int>((x >> (bit - 2)) & 3);
            for(int i = vb + 1; i < 4; ++i){
                uint32_t c = nodes[pos].child[i ^ xb];
                if(c != 0){
                    cand = c;
                    cand_bit = bit;
                    break;
                }
            }
            pos = nodes[pos].child[vb ^ xb];
            if(pos == 0){
                return cand ? ref_node_t{_min_from(cand, cand_bit, xor_val), true} : ref_node_t{0, false};
            }
        }
    }
    
    ref_node_t max_leq(T x) const {
        if(root == 0) return ref_node_t{0, false};
        uint32_t cand = 0;
        int cand_bit = 0;
        uint32_t pos = root;
        int bit = bit_width;
        while(true){
            int w = nodes[pos].width;
            int nbit = bit - w;
            T pv_eff = nodes[pos].value ^ xor_val;
            T x_xor = (w == 0) ? T(0) : static_cast<T>((pv_eff ^ x) >> nbit);
            if(x_xor != 0){
                int d = bit_width - clz(x_xor) + nbit;
                if(((pv_eff >> (d - 1)) & 1) == 0) return ref_node_t{_max_from(pos, bit, xor_val), true};
                return cand ? ref_node_t{_max_from(cand, cand_bit, xor_val), true} : ref_node_t{0, false};
            }
            bit = nbit;
            if(bit == 0) return ref_node_t{x, true};
            int xb = static_cast<int>((xor_val >> (bit - 2)) & 3);
            int vb = static_cast<int>((x >> (bit - 2)) & 3);
            for(int i = vb - 1; i >= 0; --i){
                uint32_t c = nodes[pos].child[i ^ xb];
                if(c != 0){
                    cand = c;
                    cand_bit = bit;
                    break;
                }
            }
            pos = nodes[pos].child[vb ^ xb];
            if(pos == 0){
                return cand ? ref_node_t{_max_from(cand, cand_bit, xor_val), true} : ref_node_t{0, false};
            }
        }
    }
    
    int size() const {
        return siz;
    }
    
    void xor_all(T x) {
        xor_val ^= x;
    }
    
    T xor_min(T x) const {
        assert(siz > 0);
        return _min_from(root, bit_width, xor_val ^ x);
    }
    
    T xor_max(T x) const {
        assert(siz > 0);
        return _max_from(root, bit_width, xor_val ^ x);
    }
};
