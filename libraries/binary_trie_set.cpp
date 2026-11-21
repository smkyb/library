// 注意: uint32_t 専用・重複無視 (std::set 仕様、insert/erase は bool 返却)
//       xor_min(x) / xor_max(x) は v^x の値ではなく最小/最大にする要素 v 自体を返す
//       max_leq(x) は x 以下 (x を含む) の最大値; lower_bound/max_leq は *r, .value_or(def), if(r) 対応
struct binary_trie_set{
    struct node_t{
        uint32_t value = 0;
        int count = 0;
        uint16_t exist_mask = 0;
        uint16_t leaf_mask = 0;
        uint8_t width = 0;
        uint32_t child[16] = {};
        node_t() = default;
    };
    
    struct ref_node_t{
        uint32_t val;
        bool exist;
        explicit operator bool() const { return exist; }
        uint32_t operator*() const { assert(exist); return val; }
        template<typename U = uint32_t>
        U value_or(U def) const { return exist ? static_cast<U>(val) : def; }
    };
    
    vector<node_t> nodes;
    uint32_t root = 0;
    uint32_t free_head = 0;
    uint32_t single_val = 0;
    uint32_t xor_val = 0;
    int siz = 0;
    
    static inline uint16_t perm16(uint16_t m, int xb) {
        if(__builtin_expect(xb != 0, 0)){
            if(xb & 1) m = static_cast<uint16_t>(((m & 0x5555u) << 1) | ((m & 0xAAAAu) >> 1));
            if(xb & 2) m = static_cast<uint16_t>(((m & 0x3333u) << 2) | ((m & 0xCCCCu) >> 2));
            if(xb & 4) m = static_cast<uint16_t>(((m & 0x0F0Fu) << 4) | ((m & 0xF0F0u) >> 4));
            if(xb & 8) m = static_cast<uint16_t>(((m & 0x00FFu) << 8) | ((m & 0xFF00u) >> 8));
        }
        return m;
    }
    
    inline uint32_t alloc_node(){
        if(__builtin_expect(free_head != 0, 0)){
            uint32_t id = free_head;
            free_head = nodes[id].child[0];
            return id;
        }
        nodes.emplace_back();
        return static_cast<uint32_t>(nodes.size() - 1);
    }
    
    inline void push_free(uint32_t id){
        nodes[id].child[0] = free_head;
        free_head = id;
    }
    
    uint32_t _min_from(uint32_t pos, int bit, uint32_t xv) const {
        while(true){
            bit -= nodes[pos].width;
            int xb = static_cast<int>((xv >> (bit - 4)) & 15u);
            uint16_t em = perm16(nodes[pos].exist_mask, xb);
            int slot = __builtin_ctz(em) ^ xb;
            if(nodes[pos].leaf_mask & (1u << slot)) return nodes[pos].child[slot] ^ xor_val;
            pos = nodes[pos].child[slot];
        }
    }
    
    uint32_t _max_from(uint32_t pos, int bit, uint32_t xv) const {
        while(true){
            bit -= nodes[pos].width;
            int xb = static_cast<int>((xv >> (bit - 4)) & 15u);
            uint16_t em = perm16(nodes[pos].exist_mask, xb);
            int slot = (31 - __builtin_clz(em)) ^ xb;
            if(nodes[pos].leaf_mask & (1u << slot)) return nodes[pos].child[slot] ^ xor_val;
            pos = nodes[pos].child[slot];
        }
    }
    
    public:
    
    binary_trie_set() : nodes(1) {}
    binary_trie_set(const binary_trie_set&) = default;
    binary_trie_set& operator=(const binary_trie_set&) = default;
    binary_trie_set(binary_trie_set&&) noexcept = default;
    binary_trie_set& operator=(binary_trie_set&&) noexcept = default;
    
    void reserve(int n){
        nodes.reserve(static_cast<size_t>(n) + 2);
    }
    
    bool insert(uint32_t x) {
        x ^= xor_val;
        if(__builtin_expect(siz <= 1, 0)){
            if(siz == 0){
                single_val = x;
                siz = 1;
                return true;
            }
            uint32_t diff = x ^ single_val;
            if(diff == 0) return false;
            int d = (32 - __builtin_clz(diff) + 3) & ~3;
            uint32_t id = alloc_node();
            int b_new = static_cast<int>((x >> (d - 4)) & 15u);
            int b_old = static_cast<int>((single_val >> (d - 4)) & 15u);
            nodes[id].value = single_val;
            nodes[id].width = static_cast<uint8_t>(32 - d);
            nodes[id].count = 2;
            nodes[id].child[b_new] = x;
            nodes[id].child[b_old] = single_val;
            nodes[id].exist_mask = static_cast<uint16_t>((1u << b_new) | (1u << b_old));
            nodes[id].leaf_mask = nodes[id].exist_mask;
            root = id;
            siz = 2;
            return true;
        }
        uint32_t path[9];
        int depth = 0;
        uint32_t pos = root;
        int bit = 32;
        while(true){
            int w = nodes[pos].width;
            int nbit = bit - w;
            uint32_t pv = nodes[pos].value;
            uint32_t x_xor = (w == 0) ? 0u : ((x ^ pv) >> nbit);
            if(x_xor != 0){
                int d = (32 - __builtin_clz(x_xor) + nbit + 3) & ~3;
                node_t old_nd = nodes[pos];
                uint32_t inter = alloc_node();
                nodes[inter] = old_nd;
                nodes[inter].width = static_cast<uint8_t>(w - (bit - d));
                int b_new = static_cast<int>((x >> (d - 4)) & 15u);
                int b_old = static_cast<int>((pv >> (d - 4)) & 15u);
                nodes[pos].width = static_cast<uint8_t>(bit - d);
                nodes[pos].count = old_nd.count + 1;
                nodes[pos].child[b_new] = x;
                nodes[pos].child[b_old] = inter;
                nodes[pos].exist_mask = static_cast<uint16_t>((1u << b_new) | (1u << b_old));
                nodes[pos].leaf_mask = static_cast<uint16_t>(1u << b_new);
                siz++;
                return true;
            }
            path[depth++] = pos;
            nodes[pos].count++;
            bit = nbit;
            int b = static_cast<int>((x >> (bit - 4)) & 15u);
            uint16_t bm = static_cast<uint16_t>(1u << b);
            if(!(nodes[pos].exist_mask & bm)){
                nodes[pos].child[b] = x;
                nodes[pos].exist_mask |= bm;
                nodes[pos].leaf_mask |= bm;
                siz++;
                return true;
            }
            if(nodes[pos].leaf_mask & bm){
                uint32_t lv = nodes[pos].child[b];
                uint32_t diff = x ^ lv;
                if(diff == 0){
                    for(int i = 0; i < depth; ++i) nodes[path[i]].count--;
                    return false;
                }
                int d = (32 - __builtin_clz(diff) + 3) & ~3;
                uint32_t inter = alloc_node();
                int b_new = static_cast<int>((x >> (d - 4)) & 15u);
                int b_old = static_cast<int>((lv >> (d - 4)) & 15u);
                nodes[inter].value = lv;
                nodes[inter].width = static_cast<uint8_t>(bit - d);
                nodes[inter].count = 2;
                nodes[inter].child[b_new] = x;
                nodes[inter].child[b_old] = lv;
                nodes[inter].exist_mask = static_cast<uint16_t>((1u << b_new) | (1u << b_old));
                nodes[inter].leaf_mask = nodes[inter].exist_mask;
                nodes[pos].child[b] = inter;
                nodes[pos].leaf_mask &= static_cast<uint16_t>(~bm);
                siz++;
                return true;
            }
            pos = nodes[pos].child[b];
        }
    }
    
    bool contains(uint32_t x) const {
        if(siz == 0) return false;
        uint32_t key = x ^ xor_val;
        if(siz == 1) return single_val == key;
        uint32_t pos = root;
        int bit = 32;
        while(true){
            int w = nodes[pos].width;
            int nbit = bit - w;
            uint32_t x_xor = (w == 0) ? 0u : ((key ^ nodes[pos].value) >> nbit);
            if(x_xor != 0) return false;
            bit = nbit;
            int b = static_cast<int>((key >> (bit - 4)) & 15u);
            uint16_t bm = static_cast<uint16_t>(1u << b);
            if(!(nodes[pos].exist_mask & bm)) return false;
            if(nodes[pos].leaf_mask & bm) return nodes[pos].child[b] == key;
            pos = nodes[pos].child[b];
        }
    }
    
    int count(uint32_t x) const {
        return contains(x) ? 1 : 0;
    }
    
    bool erase(uint32_t x) {
        if(siz == 0) return false;
        uint32_t key = x ^ xor_val;
        if(siz == 1){
            if(single_val != key) return false;
            siz = 0;
            return true;
        }
        uint32_t path[9];
        uint8_t dir[9];
        int depth = 0;
        uint32_t pos = root;
        int bit = 32;
        while(true){
            int w = nodes[pos].width;
            int nbit = bit - w;
            uint32_t x_xor = (w == 0) ? 0u : ((key ^ nodes[pos].value) >> nbit);
            if(x_xor != 0) return false;
            bit = nbit;
            int b = static_cast<int>((key >> (bit - 4)) & 15u);
            uint16_t bm = static_cast<uint16_t>(1u << b);
            if(!(nodes[pos].exist_mask & bm)) return false;
            if(nodes[pos].leaf_mask & bm){
                if(nodes[pos].child[b] != key) return false;
                siz--;
                nodes[pos].exist_mask &= static_cast<uint16_t>(~bm);
                nodes[pos].leaf_mask &= static_cast<uint16_t>(~bm);
                nodes[pos].count--;
                if(__builtin_popcount(nodes[pos].exist_mask) == 1){
                    int s = __builtin_ctz(nodes[pos].exist_mask);
                    uint32_t rem = nodes[pos].child[s];
                    bool rem_leaf = (nodes[pos].leaf_mask & (1u << s)) != 0;
                    uint8_t pw = nodes[pos].width;
                    push_free(pos);
                    if(depth == 0){
                        if(rem_leaf){
                            single_val = rem;
                            root = 0;
                        } else {
                            nodes[rem].width = static_cast<uint8_t>(nodes[rem].width + pw);
                            root = rem;
                        }
                        return true;
                    }
                    uint32_t p = path[depth - 1];
                    int pd = dir[depth - 1];
                    nodes[p].child[pd] = rem;
                    if(rem_leaf){
                        nodes[p].leaf_mask |= static_cast<uint16_t>(1u << pd);
                    } else {
                        nodes[rem].width = static_cast<uint8_t>(nodes[rem].width + pw);
                    }
                }
                for(int i = 0; i < depth; ++i) nodes[path[i]].count--;
                return true;
            }
            path[depth] = pos;
            dir[depth] = static_cast<uint8_t>(b);
            ++depth;
            pos = nodes[pos].child[b];
        }
    }
    
    uint32_t operator[](int k) const {
        if(k < 0) k += siz;
        assert(0 <= k && k < siz);
        if(siz == 1) return single_val ^ xor_val;
        k++;
        uint32_t pos = root;
        int bit = 32;
        while(true){
            bit -= nodes[pos].width;
            int xb = static_cast<int>((xor_val >> (bit - 4)) & 15u);
            uint16_t em = perm16(nodes[pos].exist_mask, xb);
            uint16_t lm = perm16(nodes[pos].leaf_mask, xb);
            while(true){
                int i = __builtin_ctz(em);
                em &= static_cast<uint16_t>(em - 1);
                int slot = i ^ xb;
                if((lm >> i) & 1){
                    if(k == 1) return nodes[pos].child[slot] ^ xor_val;
                    --k;
                } else {
                    uint32_t c = nodes[pos].child[slot];
                    int cnt = nodes[c].count;
                    if(k <= cnt){
                        pos = c;
                        break;
                    }
                    k -= cnt;
                }
            }
        }
    }
    
    uint32_t min() const {
        assert(siz > 0);
        if(siz == 1) return single_val ^ xor_val;
        return _min_from(root, 32, xor_val);
    }
    
    uint32_t max() const {
        assert(siz > 0);
        if(siz == 1) return single_val ^ xor_val;
        return _max_from(root, 32, xor_val);
    }
    
    int count_less(uint32_t x) const {
        if(siz == 0) return 0;
        if(siz == 1) return (single_val ^ xor_val) < x ? 1 : 0;
        int res = 0;
        uint32_t pos = root;
        int bit = 32;
        while(true){
            int w = nodes[pos].width;
            int nbit = bit - w;
            uint32_t pv_eff = nodes[pos].value ^ xor_val;
            uint32_t x_xor = (w == 0) ? 0u : ((pv_eff ^ x) >> nbit);
            if(x_xor != 0){
                int d = 32 - __builtin_clz(x_xor) + nbit;
                if(((pv_eff >> (d - 1)) & 1u) == 0) res += nodes[pos].count;
                return res;
            }
            bit = nbit;
            int xb = static_cast<int>((xor_val >> (bit - 4)) & 15u);
            int vb = static_cast<int>((x >> (bit - 4)) & 15u);
            uint16_t lo_mask = static_cast<uint16_t>((1u << vb) - 1u);
            uint16_t lt_exist = perm16(nodes[pos].exist_mask, xb) & lo_mask;
            if(lt_exist != 0){
                uint16_t lt_leaf = lt_exist & perm16(nodes[pos].leaf_mask, xb);
                res += __builtin_popcount(lt_leaf);
                uint16_t lt_int = static_cast<uint16_t>(lt_exist & ~lt_leaf);
                while(lt_int != 0){
                    int i = __builtin_ctz(lt_int);
                    lt_int &= static_cast<uint16_t>(lt_int - 1);
                    res += nodes[nodes[pos].child[i ^ xb]].count;
                }
            }
            int slot = vb ^ xb;
            uint16_t sm = static_cast<uint16_t>(1u << slot);
            if(!(nodes[pos].exist_mask & sm)) return res;
            if(nodes[pos].leaf_mask & sm){
                if((nodes[pos].child[slot] ^ xor_val) < x) res++;
                return res;
            }
            pos = nodes[pos].child[slot];
        }
    }
    
    ref_node_t lower_bound(uint32_t x) const {
        if(siz == 0) return ref_node_t{0, false};
        if(siz == 1){
            uint32_t cur = single_val ^ xor_val;
            return cur >= x ? ref_node_t{cur, true} : ref_node_t{0, false};
        }
        bool has_cand = false;
        bool cand_is_leaf = false;
        uint32_t cand = 0;
        int cand_bit = 0;
        auto resolve_cand = [&]() -> ref_node_t {
            if(!has_cand) return ref_node_t{0, false};
            if(cand_is_leaf) return ref_node_t{cand ^ xor_val, true};
            return ref_node_t{_min_from(cand, cand_bit, xor_val), true};
        };
        uint32_t pos = root;
        int bit = 32;
        while(true){
            int w = nodes[pos].width;
            int nbit = bit - w;
            uint32_t pv_eff = nodes[pos].value ^ xor_val;
            uint32_t x_xor = (w == 0) ? 0u : ((pv_eff ^ x) >> nbit);
            if(x_xor != 0){
                int d = 32 - __builtin_clz(x_xor) + nbit;
                if((pv_eff >> (d - 1)) & 1u) return ref_node_t{_min_from(pos, bit, xor_val), true};
                return resolve_cand();
            }
            bit = nbit;
            int xb = static_cast<int>((xor_val >> (bit - 4)) & 15u);
            int vb = static_cast<int>((x >> (bit - 4)) & 15u);
            uint16_t em = perm16(nodes[pos].exist_mask, xb);
            uint16_t hi = static_cast<uint16_t>(em & ~((1u << (vb + 1)) - 1u));
            if(hi != 0){
                int slot = __builtin_ctz(hi) ^ xb;
                has_cand = true;
                cand_is_leaf = (nodes[pos].leaf_mask & (1u << slot)) != 0;
                cand = nodes[pos].child[slot];
                cand_bit = bit;
            }
            int slot = vb ^ xb;
            uint16_t sm = static_cast<uint16_t>(1u << slot);
            if(!(nodes[pos].exist_mask & sm)) return resolve_cand();
            if(nodes[pos].leaf_mask & sm){
                uint32_t lv_eff = nodes[pos].child[slot] ^ xor_val;
                if(lv_eff >= x) return ref_node_t{lv_eff, true};
                return resolve_cand();
            }
            pos = nodes[pos].child[slot];
        }
    }
    
    ref_node_t max_leq(uint32_t x) const {
        if(siz == 0) return ref_node_t{0, false};
        if(siz == 1){
            uint32_t cur = single_val ^ xor_val;
            return cur <= x ? ref_node_t{cur, true} : ref_node_t{0, false};
        }
        bool has_cand = false;
        bool cand_is_leaf = false;
        uint32_t cand = 0;
        int cand_bit = 0;
        auto resolve_cand = [&]() -> ref_node_t {
            if(!has_cand) return ref_node_t{0, false};
            if(cand_is_leaf) return ref_node_t{cand ^ xor_val, true};
            return ref_node_t{_max_from(cand, cand_bit, xor_val), true};
        };
        uint32_t pos = root;
        int bit = 32;
        while(true){
            int w = nodes[pos].width;
            int nbit = bit - w;
            uint32_t pv_eff = nodes[pos].value ^ xor_val;
            uint32_t x_xor = (w == 0) ? 0u : ((pv_eff ^ x) >> nbit);
            if(x_xor != 0){
                int d = 32 - __builtin_clz(x_xor) + nbit;
                if(((pv_eff >> (d - 1)) & 1u) == 0) return ref_node_t{_max_from(pos, bit, xor_val), true};
                return resolve_cand();
            }
            bit = nbit;
            int xb = static_cast<int>((xor_val >> (bit - 4)) & 15u);
            int vb = static_cast<int>((x >> (bit - 4)) & 15u);
            uint16_t em = perm16(nodes[pos].exist_mask, xb);
            uint16_t lo = static_cast<uint16_t>(em & ((1u << vb) - 1u));
            if(lo != 0){
                int slot = (31 - __builtin_clz(lo)) ^ xb;
                has_cand = true;
                cand_is_leaf = (nodes[pos].leaf_mask & (1u << slot)) != 0;
                cand = nodes[pos].child[slot];
                cand_bit = bit;
            }
            int slot = vb ^ xb;
            uint16_t sm = static_cast<uint16_t>(1u << slot);
            if(!(nodes[pos].exist_mask & sm)) return resolve_cand();
            if(nodes[pos].leaf_mask & sm){
                uint32_t lv_eff = nodes[pos].child[slot] ^ xor_val;
                if(lv_eff <= x) return ref_node_t{lv_eff, true};
                return resolve_cand();
            }
            pos = nodes[pos].child[slot];
        }
    }
    
    int size() const {
        return siz;
    }
    
    void xor_all(uint32_t x) {
        xor_val ^= x;
    }
    
    uint32_t xor_min(uint32_t x) const {
        assert(siz > 0);
        if(siz == 1) return single_val ^ xor_val;
        return _min_from(root, 32, xor_val ^ x);
    }
    
    uint32_t xor_max(uint32_t x) const {
        assert(siz > 0);
        if(siz == 1) return single_val ^ xor_val;
        return _max_from(root, 32, xor_val ^ x);
    }
};
