// 注意: C++20・GCC/Clang 専用; 可換モノイド必須 (op は結合則・可換則が必要)
//       prod(lx, ly, rx, ry) の [ly, ry) は int 半開区間のため Y=INT_MAX は右端に含められない; X は 0 <= x < bit_ceil(n)
template<typename S, auto op, auto e>
struct segtree_2d{
    private:
    
    struct node_t{
        S sum;
        uint32_t value = 0;
        uint32_t child[2] = {0, 0};
        uint8_t width = 0;
        node_t() : sum{e()} {}
        node_t(uint32_t v, S x, uint8_t w, uint32_t c0 = 0, uint32_t c1 = 0)
            : sum(move(x)), value(v), child{c0, c1}, width(w) {}
    };
    
    int siz;
    vector<uint32_t> roots;
    vector<node_t> pool;
    
    static constexpr uint32_t to_u(int y) noexcept {
        return static_cast<uint32_t>(y) ^ (1u << 31);
    }
    
    inline uint32_t make_node(uint32_t v, S x, uint8_t w, uint32_t c0 = 0, uint32_t c1 = 0){
        pool.emplace_back(v, move(x), w, c0, c1);
        return static_cast<uint32_t>(pool.size() - 1);
    }
    
    template<bool is_add>
    void _update(uint32_t &rt, uint32_t v, const S &x){
        if(rt == 0){
            rt = make_node(v, x, 32);
            return;
        }
        uint32_t route[33];
        int rcnt = 0;
        uint32_t pos = rt;
        int bit = 32;
        while(true){
            int w = pool[pos].width;
            int nbit = bit - w;
            uint32_t x_xor = (w == 0) ? 0u : ((v ^ pool[pos].value) >> nbit);
            if(x_xor != 0){
                int d = 32 - __builtin_clz(x_xor) + nbit;
                uint32_t pv = pool[pos].value;
                S ps = move(pool[pos].sum);
                uint32_t c0 = pool[pos].child[0], c1 = pool[pos].child[1];
                uint8_t iw = static_cast<uint8_t>(w - (bit - d));
                uint32_t inter = make_node(pv, move(ps), iw, c0, c1);
                uint32_t leaf = make_node(v, x, static_cast<uint8_t>(d));
                int b = static_cast<int>((v >> (d - 1)) & 1u);
                pool[pos].width = static_cast<uint8_t>(bit - d);
                pool[pos].child[b] = leaf;
                pool[pos].child[b ^ 1] = inter;
                route[rcnt++] = pos;
                break;
            }
            bit = nbit;
            if(bit == 0){
                if constexpr (is_add) pool[pos].sum = op(pool[pos].sum, x);
                else pool[pos].sum = x;
                break;
            }
            route[rcnt++] = pos;
            pos = pool[pos].child[(v >> (bit - 1)) & 1u];
        }
        for(int i = rcnt - 1; i >= 0; --i){
            uint32_t idx = route[i];
            pool[idx].sum = op(pool[pool[idx].child[0]].sum, pool[pool[idx].child[1]].sum);
        }
    }
    
    S _get(uint32_t pos, uint32_t p) const {
        if(pos == 0) return e();
        int bit = 32;
        while(true){
            bit -= pool[pos].width;
            if(bit == 0) return pool[pos].value == p ? pool[pos].sum : e();
            pos = pool[pos].child[(p >> (bit - 1)) & 1u];
        }
    }
    
    S _prod(uint32_t l, uint32_t r, uint32_t pos, int bit) const {
        int nbit = bit - pool[pos].width;
        uint32_t mv, hi;
        if(nbit == 32){
            mv = 0u;
            hi = ~0u;
        } else {
            uint32_t lm = (nbit == 0) ? 0u : ((1u << nbit) - 1u);
            mv = pool[pos].value & ~lm;
            hi = pool[pos].value | lm;
        }
        if(r <= mv || hi < l) return e();
        if(l <= mv && hi < r) return pool[pos].sum;
        uint32_t mid = mv | (1u << (nbit - 1));
        if(r <= mid) return _prod(l, r, pool[pos].child[0], nbit);
        if(l >= mid) return _prod(l, r, pool[pos].child[1], nbit);
        return op(_prod(l, r, pool[pos].child[0], nbit), _prod(l, r, pool[pos].child[1], nbit));
    }
    
    public:
    
    segtree_2d(int n = 0) : siz((assert(n >= 0), bit_ceil(unsigned(n)))), roots(siz * 2, 0), pool(1) {}
    
    void reserve(int n){
        pool.reserve(static_cast<size_t>(n) * 32 + 2);
    }
    
    void set(int x, int y, const S &val){
        assert(0 <= x && x < siz);
        x += siz;
        uint32_t uy = to_u(y);
        _update<false>(roots[x], uy, val);
        while(x >>= 1){
            uint32_t r0 = roots[x << 1], r1 = roots[(x << 1) | 1];
            if(r0 && r1) _update<false>(roots[x], uy, op(_get(r0, uy), _get(r1, uy)));
            else if(r0) _update<false>(roots[x], uy, _get(r0, uy));
            else _update<false>(roots[x], uy, _get(r1, uy));
        }
    }
    
    void add(int x, int y, const S &val){
        assert(0 <= x && x < siz);
        x += siz;
        uint32_t uy = to_u(y);
        _update<true>(roots[x], uy, val);
        while(x >>= 1) _update<true>(roots[x], uy, val);
    }
    
    S get(int x, int y) const {
        assert(0 <= x && x < siz);
        return _get(roots[x + siz], to_u(y));
    }
    
    S prod(int lx, int ly, int rx, int ry) const {
        lx = max(0, lx); rx = min(rx, siz);
        if(lx >= rx || ly >= ry) return e();
        uint32_t uly = to_u(ly), ury = to_u(ry);
        S l_ans = e(), r_ans = e();
        for(lx += siz, rx += siz; lx < rx; lx >>= 1, rx >>= 1){
            if(lx & 1){
                if(roots[lx]) l_ans = op(l_ans, _prod(uly, ury, roots[lx], 32));
                ++lx;
            }
            if(rx & 1){
                --rx;
                if(roots[rx]) r_ans = op(_prod(uly, ury, roots[rx], 32), r_ans);
            }
        }
        return op(l_ans, r_ans);
    }
};