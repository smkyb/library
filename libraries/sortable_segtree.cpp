// 注意: 全要素でキー相異なること (構築時・set() 後とも重複キー非対応); キー型 T は uint32_t / uint64_t のみ
//       要素数 n <= 524287 (fastset::MAXN=524288 に対し n+1 使用); 64bit GCC/Clang (__uint128_t)・C++20 必須
//       コピー不可 (ムーブのみ)・静的メモリプール共有; prod/sort/sort_desc は半開 [l, r) (範囲チェックなし)
//       非可換モノイド対応 (op が可換なら commutative = true で高速・省メモリ化)
namespace smkyb {

template<typename T, typename S, auto op, auto e, typename LeafS = S, auto lift = [](const S &x) -> const S& { return x; }>
struct meldable_binary_trie{
    static_assert(is_unsigned_v<T> && (sizeof(T) == 4 || sizeof(T) == 8));
    
    static constexpr int bit_width = sizeof(T) * 8;
    static constexpr uint32_t NIL = 0;
    static constexpr uint32_t LEAF_BIT = 0x80000000u;
    
    static inline bool is_leaf(uint32_t id) noexcept { return (id & LEAF_BIT) != 0; }
    static inline uint32_t leaf_idx(uint32_t id) noexcept { return id & ~LEAF_BIT; }
    static inline uint32_t make_leaf_id(uint32_t idx) noexcept { return idx | LEAF_BIT; }
    
    struct leaf_t{
        LeafS val;
        T value;
        leaf_t() = default;
        leaf_t(T v, const LeafS &x) : val(x), value(v) {}
    };
    
    static constexpr size_t INODE_ALIGN =
        (sizeof(S) + sizeof(T) + 12 <= 32 && alignof(S) <= 32 && alignof(T) <= 32)
            ? 32
            : (alignof(S) > alignof(T) ? alignof(S) : alignof(T));
    
    struct alignas(INODE_ALIGN) inode_t{
        S sum;
        T value;
        uint32_t child[2];
        // bits [0..6]: split height (1..bit_width), bits [7..31]: left subtree count (lcnt)
        uint32_t ch;
        inode_t() = default;
        inode_t(T v, const S &s, uint32_t h, uint32_t lc, uint32_t c0, uint32_t c1)
            : sum(s), value(v), child{c0, c1}, ch((lc << 7) | h) {}
        inline uint32_t get_h() const noexcept { return ch & 127u; }
        inline int get_lcnt() const noexcept { return static_cast<int>(ch >> 7); }
        inline void add_lcnt(int d) noexcept { ch += static_cast<uint32_t>(d) << 7; }
        inline void sub_lcnt(int d) noexcept { ch -= static_cast<uint32_t>(d) << 7; }
        inline void dec_lcnt() noexcept { ch -= 128u; }
        inline void set_lcnt(int c) noexcept { ch = (ch & 127u) | (static_cast<uint32_t>(c) << 7); }
    };
    
    struct Pool {
        vector<inode_t> inodes;
        vector<leaf_t> leaves;
        uint32_t ifree = NIL;
        uint32_t lfree = NIL;
        Pool() {
            pool_alive = true;
            inodes.reserve(1 << 17);
            leaves.reserve(1 << 17);
            inodes.emplace_back(0, e(), 0, 0, NIL, NIL);
            leaves.emplace_back();
        }
        ~Pool() {
            pool_alive = false;
        }
        void reserve(size_t n_leaves) {
            if(inodes.capacity() < inodes.size() + n_leaves + 4) inodes.reserve(inodes.size() + n_leaves + 4);
            if(leaves.capacity() < leaves.size() + n_leaves + 4) leaves.reserve(leaves.size() + n_leaves + 4);
        }
        __attribute__((noinline))
        uint32_t make_inode_slow(T v, const S &s, uint32_t h, uint32_t lc, uint32_t c0, uint32_t c1) {
            uint32_t id = static_cast<uint32_t>(inodes.size());
            inodes.emplace_back(v, s, h, lc, c0, c1);
            return id;
        }
        inline uint32_t make_inode(T v, const S &s, uint32_t h, uint32_t lc, uint32_t c0, uint32_t c1) {
            if(ifree) [[likely]] {
                uint32_t id = ifree;
                ifree = inodes[id].child[0];
                inodes[id] = inode_t(v, s, h, lc, c0, c1);
                return id;
            }
            if(inodes.size() == inodes.capacity()) [[unlikely]] return make_inode_slow(v, s, h, lc, c0, c1);
            uint32_t id = static_cast<uint32_t>(inodes.size());
            inodes.emplace_back(v, s, h, lc, c0, c1);
            return id;
        }
        inline void push_inode(uint32_t id) {
            inodes[id].child[0] = ifree;
            ifree = id;
        }
        __attribute__((noinline))
        uint32_t make_leaf_slow(T v, const LeafS &x) {
            uint32_t idx = static_cast<uint32_t>(leaves.size());
            leaves.emplace_back(v, x);
            return make_leaf_id(idx);
        }
        inline uint32_t make_leaf(T v, const LeafS &x) {
            if(lfree) {
                uint32_t idx = lfree;
                lfree = static_cast<uint32_t>(leaves[idx].value);
                leaves[idx] = leaf_t(v, x);
                return make_leaf_id(idx);
            }
            if(leaves.size() == leaves.capacity()) [[unlikely]] return make_leaf_slow(v, x);
            uint32_t idx = static_cast<uint32_t>(leaves.size());
            leaves.emplace_back(v, x);
            return make_leaf_id(idx);
        }
        inline void push_leaf(uint32_t id) {
            uint32_t idx = leaf_idx(id);
            leaves[idx].value = static_cast<T>(lfree);
            lfree = idx;
        }
    };
    
    inline static bool pool_alive = false;
    inline static Pool pool;
    static inline Pool& get_pool() noexcept { return pool; }
    uint32_t root = NIL;
    uint32_t sz_rev = 0; // (sz << 1) | rev
    
    template<typename _Tp>
    inline static uint32_t msb_bit(_Tp x) {
        if constexpr(sizeof(_Tp) == 8ull) return 63u - static_cast<uint32_t>(__builtin_clzll(x));
        else return 31u - static_cast<uint32_t>(__builtin_clz(x));
    }
    
    static inline S node_sum(const inode_t *in, const leaf_t *lf, uint32_t id) noexcept {
        return is_leaf(id) ? lift(lf[leaf_idx(id)].val) : in[id].sum;
    }
    static inline void prefetch_node(const inode_t *in, uint32_t id) noexcept {
        if(!is_leaf(id)) __builtin_prefetch(&in[id], 0, 1);
    }
    
    static void _clear(uint32_t p) {
        auto &pool = get_pool();
        if(is_leaf(p)) {
            pool.push_leaf(p);
            return;
        }
        uint32_t c0 = pool.inodes[p].child[0];
        uint32_t c1 = pool.inodes[p].child[1];
        _clear(c0);
        _clear(c1);
        pool.push_inode(p);
    }
    
    public:
    
    struct LeafItem { T key; uint32_t id; };
    struct DropRes { uint32_t left, dropped, right; };
    struct Split3Res { uint32_t left, mid, right; };
    struct MNode { uint32_t rt; uint32_t h; int sz; T val; };
    
    static void reserve(size_t n) {
        get_pool().reserve(n);
    }
    
    meldable_binary_trie() = default;
    meldable_binary_trie(T p, const LeafS &x) : root(get_pool().make_leaf(p, x)), sz_rev(2u) {}
    meldable_binary_trie(uint32_t x, int sz, bool r = false)
        : root(x), sz_rev((static_cast<uint32_t>(sz) << 1) | static_cast<uint32_t>(x != NIL && r)) {}
    
    meldable_binary_trie(const meldable_binary_trie&) = delete;
    meldable_binary_trie& operator=(const meldable_binary_trie&) = delete;
    
    meldable_binary_trie(meldable_binary_trie&& o) noexcept : root(o.root), sz_rev(o.sz_rev) {
        o.root = NIL;
        o.sz_rev = 0;
    }
    meldable_binary_trie& operator=(meldable_binary_trie&& o) noexcept {
        if(this != &o) {
            clear();
            root = o.root;
            sz_rev = o.sz_rev;
            o.root = NIL;
            o.sz_rev = 0;
        }
        return *this;
    }
    ~meldable_binary_trie() {
        clear();
    }
    
    void clear() {
        if(root != NIL) {
            if(pool_alive) _clear(root);
            root = NIL;
        }
        sz_rev = 0;
    }
    
    inline bool is_rev() const noexcept { return (sz_rev & 1u) != 0; }
    inline void set_rev(bool r) noexcept { sz_rev = (sz_rev & ~1u) | static_cast<uint32_t>(r && root != NIL); }
    inline int size() const noexcept { return static_cast<int>(sz_rev >> 1); }
    
    inline uint32_t take_root() noexcept {
        uint32_t r = root;
        root = NIL;
        sz_rev = 0;
        return r;
    }
    inline pair<uint32_t, int> take_root_sz() noexcept {
        uint32_t r = root;
        int sz = static_cast<int>(sz_rev >> 1);
        root = NIL;
        sz_rev = 0;
        return {r, sz};
    }
    inline void set_root(uint32_t r, int sz, bool is_r) noexcept {
        root = r;
        sz_rev = (static_cast<uint32_t>(sz) << 1) | static_cast<uint32_t>(r != NIL && is_r);
    }
    
    static inline uint64_t pack_node(uint32_t id, int sz) noexcept {
        return (static_cast<uint64_t>(static_cast<uint32_t>(sz)) << 32) | id;
    }
    static inline MNode make_mnode(const inode_t *in, const leaf_t *lf, uint32_t rt, int sz) noexcept {
        if(is_leaf(rt)) return {rt, 0u, 1, lf[leaf_idx(rt)].value};
        return {rt, in[rt].get_h(), sz, in[rt].value};
    }
    
    static uint32_t meld_multi(Pool &pool, vector<MNode> &buf, int bg, int ed, int free_pos) {
        int k = ed - bg;
        if(k == 1) return buf[bg].rt;
        if(k == 2) return meld(pool, pack_node(buf[bg].rt, buf[bg].sz), pack_node(buf[bg + 1].rt, buf[bg + 1].sz));
        MNode *a = buf.data();
        uint32_t max_h = 0;
        int n_exp = 0;
        T v0 = a[bg].val, diff = 0;
        for(int i = bg; i < ed; ++i){
            uint32_t h = a[i].h;
            if(h > max_h){ max_h = h; n_exp = 1; }
            else if(h == max_h){ ++n_exp; }
            diff |= (v0 ^ a[i].val);
        }
        uint32_t split_b;
        bool has_exp;
        if(diff != 0){
            uint32_t b = msb_bit(diff);
            if(b >= max_h){
                split_b = b;
                has_exp = false;
            } else {
                split_b = max_h - 1u;
                has_exp = true;
            }
        } else {
            split_b = max_h - 1u;
            has_exp = true;
        }
        T bit = T(1) << split_b;
        if(!has_exp){
            int i = bg, j = ed - 1;
            int lsz = 0;
            while(i <= j){
                if((a[i].val & bit) == 0){
                    lsz += a[i].sz;
                    ++i;
                } else {
                    swap(a[i], a[j]);
                    --j;
                }
            }
            int mid = i;
            uint32_t lc = (mid - bg == 1) ? a[bg].rt : meld_multi(pool, buf, bg, mid, free_pos);
            uint32_t rc = (ed - mid == 1) ? buf[mid].rt : meld_multi(pool, buf, mid, ed, free_pos);
            auto *in = pool.inodes.data();
            auto *lf = pool.leaves.data();
            S s = op(node_sum(in, lf, lc), node_sum(in, lf, rc));
            return pool.make_inode(v0, s, split_b + 1u, static_cast<uint32_t>(lsz), lc, rc);
        }
        int tot = k + n_exp;
        int new_bg = free_pos, new_ed = free_pos + tot;
        if(static_cast<int>(buf.size()) < new_ed) buf.resize(static_cast<size_t>(new_ed));
        a = buf.data();
        auto *in = pool.inodes.data();
        auto *lf = pool.leaves.data();
        uint32_t keep = NIL;
        int lp = new_bg, rp = new_ed;
        int lsz = 0;
        for(int i = bg; i < ed; ++i){
            MNode cur = a[i];
            if(cur.h == max_h){
                uint32_t u = cur.rt;
                int c0_sz = in[u].get_lcnt();
                int c1_sz = cur.sz - c0_sz;
                uint32_t c0 = in[u].child[0], c1 = in[u].child[1];
                if(keep == NIL) keep = u;
                else pool.push_inode(u);
                lsz += c0_sz;
                a[lp++] = make_mnode(in, lf, c0, c0_sz);
                a[--rp] = make_mnode(in, lf, c1, c1_sz);
            } else if((cur.val & bit) == 0){
                lsz += cur.sz;
                a[lp++] = cur;
            } else {
                a[--rp] = cur;
            }
        }
        uint32_t lc = (lp - new_bg == 1) ? a[new_bg].rt : meld_multi(pool, buf, new_bg, lp, new_ed);
        uint32_t rc = (new_ed - lp == 1) ? buf[lp].rt : meld_multi(pool, buf, lp, new_ed, new_ed);
        in = pool.inodes.data();
        lf = pool.leaves.data();
        in[keep].child[0] = lc;
        in[keep].child[1] = rc;
        in[keep].set_lcnt(lsz);
        in[keep].sum = op(node_sum(in, lf, lc), node_sum(in, lf, rc));
        return keep;
    }
    
    static uint32_t meld(meldable_binary_trie &l, meldable_binary_trie &r){
        auto [lr, lsz] = l.take_root_sz();
        auto [rr, rsz] = r.take_root_sz();
        return meld(get_pool(), pack_node(lr, lsz), pack_node(rr, rsz));
    }
    static uint32_t meld(Pool &pool, uint32_t l, int lsz, uint32_t r, int rsz){
        return meld(pool, pack_node(l, lsz), pack_node(r, rsz));
    }
    static uint32_t meld(Pool &pool, uint64_t lp, uint64_t rp){
        uint32_t l = static_cast<uint32_t>(lp), r = static_cast<uint32_t>(rp);
        if(l == NIL) return r;
        if(r == NIL) return l;
        int lsz = static_cast<int>(lp >> 32), rsz = static_cast<int>(rp >> 32);
        uint32_t st[bit_width + 1];
        uint8_t sdir[bit_width + 1];
        int sp = 0;
        uint32_t res;
        auto *in = pool.inodes.data();
        auto *lf = pool.leaves.data();
        uint32_t lh = is_leaf(l) ? 0u : in[l].get_h();
        uint32_t rh = is_leaf(r) ? 0u : in[r].get_h();
        T lv = (lh == 0u) ? lf[leaf_idx(l)].value : in[l].value;
        T rv = (rh == 0u) ? lf[leaf_idx(r)].value : in[r].value;
        while(true){
            if(lh < rh){ swap(l, r); swap(lh, rh); swap(lsz, rsz); swap(lv, rv); }
            T x = lv ^ rv;
            if(lh == 0u || (x >> (lh - 1u)) >= 2u){
                uint32_t b = msb_bit(x);
                uint32_t dir = (rv >> b) & 1u;
                uint32_t c0 = dir ? l : r;
                uint32_t c1 = dir ? r : l;
                int c0_sz = dir ? lsz : rsz;
                S s = op(node_sum(in, lf, c0), node_sum(in, lf, c1));
                res = pool.make_inode(lv, s, b + 1u, static_cast<uint32_t>(c0_sz), c0, c1);
                break;
            }
            if(lh == rh){
                int lc0_sz = in[l].get_lcnt();
                int lc1_sz = lsz - lc0_sz;
                int rc0_sz = in[r].get_lcnt();
                int rc1_sz = rsz - rc0_sz;
                in[l].add_lcnt(rc0_sz);
                uint32_t lc0 = in[l].child[0], lc1 = in[l].child[1];
                uint32_t rc0 = in[r].child[0], rc1 = in[r].child[1];
                pool.push_inode(r);
                uint32_t nc0 = meld(pool, pack_node(lc0, lc0_sz), pack_node(rc0, rc0_sz));
                uint32_t nc1 = meld(pool, pack_node(lc1, lc1_sz), pack_node(rc1, rc1_sz));
                in = pool.inodes.data();
                lf = pool.leaves.data();
                in[l].child[0] = nc0;
                in[l].child[1] = nc1;
                in[l].sum = op(node_sum(in, lf, nc0), node_sum(in, lf, nc1));
                res = l;
                break;
            }
            int lcnt = in[l].get_lcnt();
            uint32_t dir = (rv >> (lh - 1u)) & 1u;
            in[l].add_lcnt(rsz & (static_cast<int>(dir) - 1));
            lsz = dir ? (lsz - lcnt) : lcnt;
            prefetch_node(in, in[l].child[dir ^ 1u]);
            st[sp] = l;
            sdir[sp] = static_cast<uint8_t>(dir);
            ++sp;
            l = in[l].child[dir];
            lh = is_leaf(l) ? 0u : in[l].get_h();
            lv = (lh == 0u) ? lf[leaf_idx(l)].value : in[l].value;
        }
        in = pool.inodes.data();
        lf = pool.leaves.data();
        while(sp > 0){
            --sp;
            uint32_t p = st[sp];
            in[p].child[sdir[sp]] = res;
            in[p].sum = op(node_sum(in, lf, in[p].child[0]), node_sum(in, lf, in[p].child[1]));
            res = p;
        }
        return res;
    }
    
    static uint32_t build_from_leaves(Pool &pool, vector<LeafItem> &items, vector<LeafItem> &tmp){
        int m = static_cast<int>(items.size());
        if(m == 0) return NIL;
        if(m == 1) return items[0].id;
        if constexpr (sizeof(T) == 4){
            if(m >= 2048){
                tmp.resize(m);
                uint32_t c0[2048] = {}, c1[2048] = {}, c2[1024] = {};
                const auto *src = items.data();
                for(int i = 0; i < m; ++i){
                    uint32_t k = src[i].key;
                    ++c0[k & 2047u];
                    ++c1[(k >> 11) & 2047u];
                    ++c2[k >> 22];
                }
                LeafItem *a = items.data(), *b = tmp.data();
                uint32_t um = static_cast<uint32_t>(m);
                if(c0[a[0].key & 2047u] != um){
                    uint32_t acc = 0;
                    for(int i = 0; i < 2048; ++i){ uint32_t c = c0[i]; c0[i] = acc; acc += c; }
                    for(int i = 0; i < m; ++i) b[c0[a[i].key & 2047u]++] = a[i];
                    swap(a, b);
                }
                if(c1[(a[0].key >> 11) & 2047u] != um){
                    uint32_t acc = 0;
                    for(int i = 0; i < 2048; ++i){ uint32_t c = c1[i]; c1[i] = acc; acc += c; }
                    for(int i = 0; i < m; ++i) b[c1[(a[i].key >> 11) & 2047u]++] = a[i];
                    swap(a, b);
                }
                if(c2[a[0].key >> 22] != um){
                    uint32_t acc = 0;
                    for(int i = 0; i < 1024; ++i){ uint32_t c = c2[i]; c2[i] = acc; acc += c; }
                    for(int i = 0; i < m; ++i) b[c2[a[i].key >> 22]++] = a[i];
                    swap(a, b);
                }
                if(a != items.data()) items.swap(tmp);
            } else if(m >= 128){
                tmp.resize(m);
                uint32_t cnt[4][256] = {};
                const auto *src = items.data();
                for(int i = 0; i < m; ++i){
                    uint32_t k = src[i].key;
                    ++cnt[0][k & 255u];
                    ++cnt[1][(k >> 8) & 255u];
                    ++cnt[2][(k >> 16) & 255u];
                    ++cnt[3][k >> 24];
                }
                LeafItem *a = items.data(), *b = tmp.data();
                uint32_t um = static_cast<uint32_t>(m);
                for(int p = 0; p < 4; ++p){
                    int sh = p * 8;
                    uint32_t *cp = cnt[p];
                    if(cp[(a[0].key >> sh) & 255u] == um) continue;
                    uint32_t acc = 0;
                    for(int i = 0; i < 256; ++i){ uint32_t c = cp[i]; cp[i] = acc; acc += c; }
                    for(int i = 0; i < m; ++i) b[cp[(a[i].key >> sh) & 255u]++] = a[i];
                    swap(a, b);
                }
                if(a != items.data()) items.swap(tmp);
            } else {
                std::sort(items.begin(), items.end(), [](const LeafItem &a, const LeafItem &b){ return a.key < b.key; });
            }
        } else {
            if(m >= 256){
                tmp.resize(m);
                uint32_t cnt[8][256] = {};
                const auto *src = items.data();
                for(int i = 0; i < m; ++i){
                    uint64_t k = src[i].key;
                    ++cnt[0][k & 255u];
                    ++cnt[1][(k >> 8) & 255u];
                    ++cnt[2][(k >> 16) & 255u];
                    ++cnt[3][(k >> 24) & 255u];
                    ++cnt[4][(k >> 32) & 255u];
                    ++cnt[5][(k >> 40) & 255u];
                    ++cnt[6][(k >> 48) & 255u];
                    ++cnt[7][k >> 56];
                }
                LeafItem *a = items.data(), *b = tmp.data();
                uint32_t um = static_cast<uint32_t>(m);
                for(int p = 0; p < 8; ++p){
                    int sh = p * 8;
                    uint32_t *cp = cnt[p];
                    if(cp[(a[0].key >> sh) & 255u] == um) continue;
                    uint32_t acc = 0;
                    for(int i = 0; i < 256; ++i){ uint32_t c = cp[i]; cp[i] = acc; acc += c; }
                    for(int i = 0; i < m; ++i) b[cp[(a[i].key >> sh) & 255u]++] = a[i];
                    swap(a, b);
                }
                if(a != items.data()) items.swap(tmp);
            } else {
                std::sort(items.begin(), items.end(), [](const LeafItem &a, const LeafItem &b){ return a.key < b.key; });
            }
        }
        uint32_t st_node[bit_width + 2], st_h[bit_width + 2];
        int st_sz[bit_width + 2];
        T st_key[bit_width + 2];
        S st_sum[bit_width + 2];
        auto *lf = pool.leaves.data();
        st_node[0] = items[0].id;
        st_key[0] = items[0].key;
        st_h[0] = 0;
        st_sz[0] = 1;
        st_sum[0] = lift(lf[leaf_idx(items[0].id)].val);
        int sp = 1;
        for(int i = 1; i < m; ++i){
            uint32_t bh = msb_bit(items[i - 1].key ^ items[i].key) + 1u;
            while(sp >= 2 && st_h[sp - 1] < bh){
                uint32_t rnode = st_node[sp - 1], lnode = st_node[sp - 2], ph = st_h[sp - 1];
                S s = op(st_sum[sp - 2], st_sum[sp - 1]);
                uint32_t p = pool.make_inode(st_key[sp - 2], s, ph,
                                             static_cast<uint32_t>(st_sz[sp - 2]),
                                             lnode, rnode);
                --sp;
                st_node[sp - 1] = p;
                st_sum[sp - 1] = s;
                st_sz[sp - 1] += st_sz[sp];
            }
            st_node[sp] = items[i].id;
            st_key[sp] = items[i].key;
            st_h[sp] = bh;
            st_sz[sp] = 1;
            st_sum[sp] = lift(lf[leaf_idx(items[i].id)].val);
            ++sp;
        }
        while(sp >= 2){
            uint32_t rnode = st_node[sp - 1], lnode = st_node[sp - 2], ph = st_h[sp - 1];
            S s = op(st_sum[sp - 2], st_sum[sp - 1]);
            uint32_t p = pool.make_inode(st_key[sp - 2], s, ph,
                                         static_cast<uint32_t>(st_sz[sp - 2]),
                                         lnode, rnode);
            --sp;
            st_node[sp - 1] = p;
            st_sum[sp - 1] = s;
            st_sz[sp - 1] += st_sz[sp];
        }
        return st_node[0];
    }
    
    pair<meldable_binary_trie, meldable_binary_trie> split(int p){
        auto &pool = get_pool();
        bool old_rev = is_rev();
        auto [old, sz] = take_root_sz();
        int orig_p = p;
        if(old_rev) p = sz - p;
        auto res = split(pool, old, sz, p);
        if(old_rev){
            return make_pair(meldable_binary_trie(res.second, orig_p, true),
                             meldable_binary_trie(res.first, sz - orig_p, true));
        } else {
            return make_pair(meldable_binary_trie(res.first, orig_p, false),
                             meldable_binary_trie(res.second, sz - orig_p, false));
        }
    }
    
    static pair<uint32_t, uint32_t> split(Pool &pool, uint32_t l, int sz, int p){
        auto *in = pool.inodes.data();
        auto *lf = pool.leaves.data();
        if(p == 0) return {NIL, l};
        if(p == sz) return {l, NIL};
        struct Frame { uint32_t node; uint8_t dir; };
        Frame st[bit_width + 1];
        int sp = 0;
        uint32_t rl = NIL, rr = NIL;
        while(true){
            int c0_cnt = in[l].get_lcnt();
            uint32_t c0 = in[l].child[0], c1 = in[l].child[1];
            if(p == c0_cnt){
                pool.push_inode(l);
                rl = c0; rr = c1;
                break;
            } else if(p < c0_cnt){
                in[l].sub_lcnt(p);
                st[sp++] = {l, 0};
                prefetch_node(in, c1);
                l = c0;
            } else {
                st[sp++] = {l, 1};
                p -= c0_cnt;
                prefetch_node(in, c0);
                l = c1;
            }
        }
        for(int i = sp - 1; i >= 0; --i){
            uint32_t node = st[i].node;
            if(st[i].dir == 0){
                in[node].child[0] = rr;
                uint32_t c1 = in[node].child[1];
                in[node].sum = op(node_sum(in, lf, rr), node_sum(in, lf, c1));
                rr = node;
            } else {
                in[node].child[1] = rl;
                uint32_t c0 = in[node].child[0];
                in[node].sum = op(node_sum(in, lf, c0), node_sum(in, lf, rl));
                rl = node;
            }
        }
        return {rl, rr};
    }
    
    static Split3Res split3(Pool &pool, uint32_t u, int sz, int a, int b) {
        auto *in = pool.inodes.data();
        auto *lf = pool.leaves.data();
        struct Frame { uint32_t node; uint8_t dir; };
        Frame st[bit_width + 1];
        int sp = 0;
        uint32_t rl = NIL, rm = NIL, rr = NIL;
        while(true) {
            int c0_cnt = in[u].get_lcnt();
            uint32_t c0 = in[u].child[0], c1 = in[u].child[1];
            if(b < c0_cnt) {
                in[u].sub_lcnt(b);
                st[sp++] = {u, 0};
                prefetch_node(in, c1);
                u = c0; sz = c0_cnt;
            } else if(b == c0_cnt) {
                pool.push_inode(u);
                rr = c1;
                auto res = split(pool, c0, c0_cnt, a);
                rl = res.first; rm = res.second;
                break;
            } else if(a > c0_cnt) {
                st[sp++] = {u, 1};
                prefetch_node(in, c0);
                a -= c0_cnt; b -= c0_cnt; sz -= c0_cnt;
                u = c1;
            } else if(a == c0_cnt) {
                pool.push_inode(u);
                rl = c0;
                auto res = split(pool, c1, sz - c0_cnt, b - c0_cnt);
                rm = res.first; rr = res.second;
                break;
            } else {
                auto lr = split(pool, c0, c0_cnt, a);
                auto rr2 = split(pool, c1, sz - c0_cnt, b - c0_cnt);
                rl = lr.first; rr = rr2.second;
                in[u].child[0] = lr.second;
                in[u].child[1] = rr2.first;
                in[u].set_lcnt(c0_cnt - a);
                in[u].sum = op(node_sum(in, lf, lr.second), node_sum(in, lf, rr2.first));
                rm = u;
                break;
            }
        }
        for(int i = sp - 1; i >= 0; --i){
            uint32_t node = st[i].node;
            if(st[i].dir == 0){
                in[node].child[0] = rr;
                uint32_t c1 = in[node].child[1];
                in[node].sum = op(node_sum(in, lf, rr), node_sum(in, lf, c1));
                rr = node;
            } else {
                in[node].child[1] = rl;
                uint32_t c0 = in[node].child[0];
                in[node].sum = op(node_sum(in, lf, c0), node_sum(in, lf, rl));
                rl = node;
            }
        }
        return {rl, rm, rr};
    }
    
    static DropRes split_drop_at_raw(Pool &pool, uint32_t l, int p){
        auto *in = pool.inodes.data();
        auto *lf = pool.leaves.data();
        if(is_leaf(l)) return {NIL, l, NIL};
        struct Frame { uint32_t node; uint8_t dir; };
        Frame st[bit_width + 1];
        int sp = 0;
        while(!is_leaf(l)){
            int c0_cnt = in[l].get_lcnt();
            uint32_t c0 = in[l].child[0], c1 = in[l].child[1];
            if(p < c0_cnt){
                in[l].set_lcnt(c0_cnt - p - 1);
                st[sp++] = {l, 0};
                prefetch_node(in, c1);
                l = c0;
            } else {
                st[sp++] = {l, 1};
                p -= c0_cnt;
                prefetch_node(in, c0);
                l = c1;
            }
        }
        uint32_t dropped = l, rl = NIL, rr = NIL;
        for(int i = sp - 1; i >= 0; --i){
            uint32_t node = st[i].node;
            if(st[i].dir == 0){
                uint32_t c1 = in[node].child[1];
                if(rr == NIL){
                    pool.push_inode(node);
                    rr = c1;
                } else {
                    in[node].child[0] = rr;
                    in[node].sum = op(node_sum(in, lf, rr), node_sum(in, lf, c1));
                    rr = node;
                }
            } else {
                uint32_t c0 = in[node].child[0];
                if(rl == NIL){
                    pool.push_inode(node);
                    rl = c0;
                } else {
                    in[node].child[1] = rl;
                    in[node].sum = op(node_sum(in, lf, c0), node_sum(in, lf, rl));
                    rl = node;
                }
            }
        }
        return {rl, dropped, rr};
    }
    
    meldable_binary_trie split_one(){
        if(root == NIL) return meldable_binary_trie();
        auto &pool = get_pool();
        bool old_rev = is_rev();
        auto [old, sz] = take_root_sz();
        if(old_rev) return meldable_binary_trie(split_one_dir<1>(pool, old), sz - 1, true);
        else return meldable_binary_trie(split_one_dir<0>(pool, old), sz - 1, false);
    }
    template<int DIR>
    static uint32_t split_one_dir(Pool &pool, uint32_t l){
        auto *in = pool.inodes.data();
        auto *lf = pool.leaves.data();
        if(is_leaf(l)){
            pool.push_leaf(l);
            return NIL;
        }
        uint32_t st[bit_width + 1];
        int sp = 0;
        while(!is_leaf(l)){
            if constexpr (DIR == 0) in[l].dec_lcnt();
            st[sp++] = l;
            l = in[l].child[DIR];
        }
        pool.push_leaf(l);
        uint32_t cur = NIL;
        for(int i = sp - 1; i >= 0; --i){
            uint32_t p = st[i];
            if(cur == NIL){
                cur = in[p].child[DIR ^ 1];
                pool.push_inode(p);
            } else {
                in[p].child[DIR] = cur;
                in[p].sum = op(node_sum(in, lf, in[p].child[0]), node_sum(in, lf, in[p].child[1]));
                cur = p;
            }
        }
        return cur;
    }
    
    template<typename R, auto proj, auto leaf_proj>
    static inline R node_proj(const inode_t *in, const leaf_t *lf, uint32_t id) noexcept {
        return is_leaf(id) ? leaf_proj(lf[leaf_idx(id)].val) : proj(in[id].sum);
    }
    
    template<typename R, auto proj, auto leaf_proj, auto sub_op, auto sub_e>
    static R _prod_l_proj(int n, uint32_t pos, int sz, const inode_t *in, const leaf_t *lf) {
        if(n == 0 || pos == NIL) return sub_e();
        if(n == sz) return node_proj<R, proj, leaf_proj>(in, lf, pos);
        while(true) {
            int c0 = in[pos].get_lcnt();
            uint32_t c0p = in[pos].child[0];
            if(n < c0) { pos = c0p; sz = c0; continue; }
            if(n == c0) return node_proj<R, proj, leaf_proj>(in, lf, c0p);
            uint32_t c1p = in[pos].child[1];
            R res = node_proj<R, proj, leaf_proj>(in, lf, c0p);
            n -= c0; sz -= c0; pos = c1p;
            if(n == sz) return sub_op(res, node_proj<R, proj, leaf_proj>(in, lf, pos));
            while(true) {
                int c0b = in[pos].get_lcnt();
                uint32_t c0pb = in[pos].child[0];
                if(n < c0b) { pos = c0pb; sz = c0b; }
                else if(n == c0b) return sub_op(res, node_proj<R, proj, leaf_proj>(in, lf, c0pb));
                else {
                    uint32_t c1pb = in[pos].child[1];
                    res = sub_op(res, node_proj<R, proj, leaf_proj>(in, lf, c0pb));
                    n -= c0b; sz -= c0b; pos = c1pb;
                    if(n == sz) return sub_op(res, node_proj<R, proj, leaf_proj>(in, lf, pos));
                }
            }
        }
    }
    
    template<typename R, auto proj, auto leaf_proj, auto sub_op, auto sub_e>
    static R _prod_r_proj(int n, uint32_t pos, int sz, const inode_t *in, const leaf_t *lf) {
        if(n == 0 || pos == NIL) return sub_e();
        if(n == sz) return node_proj<R, proj, leaf_proj>(in, lf, pos);
        while(true) {
            int c0 = in[pos].get_lcnt();
            int c1 = sz - c0;
            uint32_t c1p = in[pos].child[1];
            if(n < c1) { pos = c1p; sz = c1; continue; }
            if(n == c1) return node_proj<R, proj, leaf_proj>(in, lf, c1p);
            uint32_t c0p = in[pos].child[0];
            R res = node_proj<R, proj, leaf_proj>(in, lf, c1p);
            n -= c1; sz = c0; pos = c0p;
            if(n == sz) return sub_op(node_proj<R, proj, leaf_proj>(in, lf, pos), res);
            while(true) {
                int c0b = in[pos].get_lcnt();
                int c1b = sz - c0b;
                uint32_t c1pb = in[pos].child[1];
                if(n < c1b) { pos = c1pb; sz = c1b; }
                else if(n == c1b) return sub_op(node_proj<R, proj, leaf_proj>(in, lf, c1pb), res);
                else {
                    uint32_t c0pb = in[pos].child[0];
                    res = sub_op(node_proj<R, proj, leaf_proj>(in, lf, c1pb), res);
                    n -= c1b; sz = c0b; pos = c0pb;
                    if(n == sz) return sub_op(node_proj<R, proj, leaf_proj>(in, lf, pos), res);
                }
            }
        }
    }
    
    template<typename R, auto proj, auto leaf_proj, auto sub_op, auto sub_e>
    static R _prod_lr_proj(int l, int r, uint32_t pos, int sz, const inode_t *in, const leaf_t *lf) {
        if(l == r || pos == NIL) return sub_e();
        if(l == 0) return _prod_l_proj<R, proj, leaf_proj, sub_op, sub_e>(r, pos, sz, in, lf);
        if(r == sz) return _prod_r_proj<R, proj, leaf_proj, sub_op, sub_e>(r - l, pos, sz, in, lf);
        while(true) {
            int c0 = in[pos].get_lcnt();
            uint32_t c0p = in[pos].child[0], c1p = in[pos].child[1];
            if(l == c0) return _prod_l_proj<R, proj, leaf_proj, sub_op, sub_e>(r - l, c1p, sz - c0, in, lf);
            if(r == c0) return _prod_r_proj<R, proj, leaf_proj, sub_op, sub_e>(r - l, c0p, c0, in, lf);
            if(l < c0) {
                if(r < c0) { pos = c0p; sz = c0; }
                else return sub_op(_prod_r_proj<R, proj, leaf_proj, sub_op, sub_e>(c0 - l, c0p, c0, in, lf),
                                   _prod_l_proj<R, proj, leaf_proj, sub_op, sub_e>(r - c0, c1p, sz - c0, in, lf));
            } else {
                l -= c0;
                r -= c0;
                sz -= c0;
                pos = c1p;
            }
        }
    }
    
    static inline const S& _id_sum(const S &x) noexcept { return x; }
    static inline const LeafS& _id_leaf(const LeafS &x) noexcept { return x; }
    
    S prod_l(int n) const {
        const auto &p = get_pool();
        return _prod_l_proj<S, _id_sum, lift, op, e>(n, root, size(), p.inodes.data(), p.leaves.data());
    }
    S prod_r(int n) const {
        const auto &p = get_pool();
        return _prod_r_proj<S, _id_sum, lift, op, e>(n, root, size(), p.inodes.data(), p.leaves.data());
    }
    S prod_lr(int l, int r) const {
        const auto &p = get_pool();
        return _prod_lr_proj<S, _id_sum, lift, op, e>(l, r, root, size(), p.inodes.data(), p.leaves.data());
    }
    
    template<typename R, auto proj, auto sub_op, auto sub_e>
    R prod_l_as(int n) const {
        const auto &p = get_pool();
        return _prod_l_proj<R, proj, _id_leaf, sub_op, sub_e>(n, root, size(), p.inodes.data(), p.leaves.data());
    }
    template<typename R, auto proj, auto sub_op, auto sub_e>
    R prod_r_as(int n) const {
        const auto &p = get_pool();
        return _prod_r_proj<R, proj, _id_leaf, sub_op, sub_e>(n, root, size(), p.inodes.data(), p.leaves.data());
    }
    template<typename R, auto proj, auto sub_op, auto sub_e>
    R prod_lr_as(int l, int r) const {
        const auto &p = get_pool();
        return _prod_lr_proj<R, proj, _id_leaf, sub_op, sub_e>(l, r, root, size(), p.inodes.data(), p.leaves.data());
    }
    
    S all_prod() const {
        if(root == NIL) return e();
        const auto &p = get_pool();
        return node_sum(p.inodes.data(), p.leaves.data(), root);
    }
    template<auto proj, auto sub_e>
    LeafS all_prod_as() const {
        if(root == NIL) return sub_e();
        const auto &p = get_pool();
        return is_leaf(root) ? p.leaves[leaf_idx(root)].val : proj(p.inodes[root].sum);
    }
    
    void init(T p, const LeafS &x){
        if(root != NIL){
            if(is_leaf(root)){
                get_pool().leaves[leaf_idx(root)] = leaf_t(p, x);
                sz_rev = 2u;
                return;
            }
            clear();
        }
        root = get_pool().make_leaf(p, x);
        sz_rev = 2u;
    }
    
    static int get_size(uint32_t ptr){
        if(ptr == NIL) return 0;
        if(is_leaf(ptr)) return 1;
        const auto &in = get_pool().inodes;
        return 1 + get_size(in[ptr].child[0]) + get_size(in[ptr].child[1]);
    }
};

struct fastset{
    using ull = unsigned long long;
    using uint = unsigned;
    using u128 = __uint128_t;
    static constexpr int MAXN = 524288;
    
    alignas(64) ull L0[8192];
    alignas(64) ull L1[128];
    u128 top = 0;
    
    fastset(int _n, bool fill = false){
        assert(0 <= _n && _n <= MAXN);
        int w0 = (_n + 63) >> 6;
        int w1 = (w0 + 63) >> 6;
        if(!fill){
            if(w0) memset(L0, 0, sizeof(ull) * static_cast<size_t>(w0));
            if(w1) memset(L1, 0, sizeof(ull) * static_cast<size_t>(w1));
            top = 0;
        } else if(_n > 0){
            memset(L0, 0xFF, sizeof(ull) * static_cast<size_t>(w0));
            if(_n & 63) L0[w0 - 1] = (1ull << (_n & 63)) - 1ull;
            memset(L1, 0xFF, sizeof(ull) * static_cast<size_t>(w1));
            if(w0 & 63) L1[w1 - 1] = (1ull << (w0 & 63)) - 1ull;
            top = (w1 == 128) ? ~u128(0) : ((u128(1) << w1) - 1);
        }
    }
    
    inline void insert(int x){
        uint u = static_cast<uint>(x);
        uint i0 = u >> 6, i1 = u >> 12;
        L0[i0] |= 1ull << (u & 63u);
        L1[i1] |= 1ull << (i0 & 63u);
        top |= u128(1) << i1;
    }
    
    inline bool count(int x) const {
        uint u = static_cast<uint>(x);
        return (L0[u >> 6] >> (u & 63u)) & 1ull;
    }
    
    inline int less_bound(int x) const {
        uint u = static_cast<uint>(x);
        uint i0 = u >> 6, i1 = u >> 12;
        ull m0 = L0[i0] & (~0ull >> (63u - (u & 63u)));
        ull m1 = (L1[i1] << (63u - (i0 & 63u))) << 1;
        if(m0 | m1) [[likely]] {
            uint j0 = m0 ? i0 : (i0 - 1u - static_cast<uint>(__builtin_clzll(m1 | 1ull)));
            ull w = m0 ? m0 : L0[j0];
            return static_cast<int>((j0 << 6) | (63u - static_cast<uint>(__builtin_clzll(w))));
        }
        u128 mt = (top << (127u - i1)) << 1;
        ull hi = static_cast<ull>(mt >> 64);
        uint step = hi ? static_cast<uint>(__builtin_clzll(hi))
                       : (64u + static_cast<uint>(__builtin_clzll(static_cast<ull>(mt))));
        i1 = i1 - 1u - step;
        i0 = (i1 << 6) | (63u - static_cast<uint>(__builtin_clzll(L1[i1])));
        return static_cast<int>((i0 << 6) | (63u - static_cast<uint>(__builtin_clzll(L0[i0]))));
    }
    
    template<typename F>
    inline void _erase_partial(uint w, ull mask, F &&fn){
        ull bits = L0[w] & mask;
        if(!bits) return;
        L0[w] &= ~bits;
        if(L0[w] == 0){
            uint i1 = w >> 6;
            L1[i1] &= ~(1ull << (w & 63u));
            if(L1[i1] == 0) top &= ~(u128(1) << i1);
        }
        uint base = w << 6;
        while(bits){
            fn(static_cast<int>(base | static_cast<uint>(__builtin_ctzll(bits))));
            bits &= bits - 1;
        }
    }
    
    template<typename F>
    void for_each_and_erase(int l, int r, F &&fn){
        if(l >= r) return;
        uint ul = static_cast<uint>(l), ur = static_cast<uint>(r - 1);
        uint wL = ul >> 6, wR = ur >> 6;
        if(wL == wR){
            _erase_partial(wL, (~0ull << (ul & 63u)) & (~0ull >> (63u - (ur & 63u))), fn);
            return;
        }
        uint fullL = wL;
        if(ul & 63u){
            _erase_partial(wL, ~0ull << (ul & 63u), fn);
            fullL = wL + 1;
        }
        uint fullR = ((ur & 63u) == 63u) ? (wR + 1) : wR;
        if(fullL < fullR){
            uint i1L = fullL >> 6, i1R = (fullR - 1) >> 6;
            u128 active_top = top & ((~u128(0) << i1L) & (~u128(0) >> (127u - i1R)));
            auto process_i1 = [&](uint i1){
                uint lo = (i1 == i1L) ? (fullL & 63u) : 0u;
                uint hi = (i1 == i1R) ? ((fullR - 1) & 63u) : 63u;
                ull act = L1[i1] & ((~0ull << lo) & (~0ull >> (63u - hi)));
                if(!act) return;
                L1[i1] &= ~act;
                if(L1[i1] == 0) top &= ~(u128(1) << i1);
                while(act){
                    uint w = (i1 << 6) | static_cast<uint>(__builtin_ctzll(act));
                    act &= act - 1;
                    ull bits = L0[w];
                    L0[w] = 0;
                    uint base = w << 6;
                    while(bits){
                        fn(static_cast<int>(base | static_cast<uint>(__builtin_ctzll(bits))));
                        bits &= bits - 1;
                    }
                }
            };
            ull lo_top = static_cast<ull>(active_top);
            while(lo_top){
                uint i1 = static_cast<uint>(__builtin_ctzll(lo_top));
                lo_top &= lo_top - 1;
                process_i1(i1);
            }
            ull hi_top = static_cast<ull>(active_top >> 64);
            while(hi_top){
                uint i1 = 64u + static_cast<uint>(__builtin_ctzll(hi_top));
                hi_top &= hi_top - 1;
                process_i1(i1);
            }
        }
        if(fullR == wR){
            _erase_partial(wR, ~0ull >> (63u - (ur & 63u)), fn);
        }
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
        siz = bit_ceil(v.size()); node.assign(siz*2, e());
        for(int i = 0; i < (int)v.size(); i++) node[i+siz] = v[i];
        build();
    }
    void build(){
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
    
    void propagate_sorted_leaves(vector<int>& buf) {
        // All input nodes lie at the same depth and are sorted by position.
        int k = static_cast<int>(buf.size());
        if (!k) return;
        int *d = buf.data();
        auto pull = [&](int p) { node[p] = op(node[p<<1], node[(p<<1)|1]); };
        while (k > 3) {
            int m=0, prev=-1;
            for (int i=0; i<k; ++i) {
                int p=d[i] >> 1;
                if (p!=prev) { d[m++]=prev=p; pull(p); }
            }
            k=m;
        }
        int a=d[0], c=d[k-1];
        if (k==3) {
            int b=d[1];
            while (a!=b && b!=c) {
                a>>=1; b>>=1; c>>=1;
                pull(a); if (a!=b) pull(b); if (b!=c) pull(c);
            }
        }
        while (a!=c) { a>>=1; c>>=1; pull(a); if (a!=c) pull(c); }
        while ((a>>=1)!=0) pull(a);
        buf.resize(1); d[0]=1;
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

// キーは全要素で相異なること。構築時だけでなく set() 後も重複キーは非対応。
// ブロックが少ない間は外側セグ木を更新せず、prod は各 trie の積を左から合成する。
// seg は内部キャッシュのため直接読まないこと。全区間の積は prod(0, n) で取得する。
template<typename T, typename S, auto op, auto e, bool commutative = false>
struct sortable_segtree {
    struct SS {
        S ltor, rtol;
        SS() = default;
        SS(const S &o) : ltor(o), rtol(o) {}
        SS(const S &l, const S &r) : ltor(l), rtol(r) {}
        static SS SS_op(const SS &l, const SS &r) {return SS{op(l.ltor, r.ltor), op(r.rtol, l.rtol)};}
        static SS SS_e() {return SS{e()};}
    };
    using TrieVal = conditional_t<commutative, S, SS>;
    static TrieVal trie_op(const TrieVal &l, const TrieVal &r) {
        if constexpr(commutative) return op(l, r);
        else return SS::SS_op(l, r);
    }
    static TrieVal trie_e() {
        if constexpr(commutative) return e();
        else return SS::SS_e();
    }
    static TrieVal lift_leaf(const S &x) {
        if constexpr(commutative) return x;
        else return SS{x, x};
    }
    static const S& proj_ltor(const TrieVal &v) {
        if constexpr(commutative) return v;
        else return v.ltor;
    }
    static const S& proj_rtol(const TrieVal &v) {
        if constexpr(commutative) return v;
        else return v.rtol;
    }
    static S op_rev(const S &l, const S &r) {
        if constexpr(commutative) return op(l, r);
        else return op(r, l);
    }
    using Trie = smkyb::meldable_binary_trie<T, TrieVal, trie_op, trie_e, S, lift_leaf>;
    struct MergeItem { uint32_t rt; int sz; };
    
    // Keep the dense tree as a snapshot while there are few sorted runs.
    static constexpr int sparse_low = 48, sparse_high = 2 * sparse_low;
    int runs = 0;
    bool sparse = false;
    vector<int> frozen_starts;
    int n;
    vector<Trie> trie;
    smkyb::fastset fset;
    // 内部キャッシュ。直接読まず、区間積には prod(l,r) を使用すること。
    smkyb::segtree<S, op, e> seg;
    vector<int> buf;
    vector<MergeItem> merge_roots;
    vector<typename Trie::MNode> mbuf;
    vector<typename Trie::LeafItem> leaf_buf, leaf_tmp;
    
    sortable_segtree(const vector<pair<T, S>> &v) : n(v.size()), trie(n), fset(n+1, true), seg(n) {
        Trie::reserve(static_cast<size_t>(n));
        auto &pool = Trie::get_pool();
        for(int i = 0; i < n; i++) trie[i].set_root(pool.make_leaf(v[i].first, v[i].second), 1, false);
        for(int i = 0; i < n; i++) seg.node[i + seg.siz] = v[i].second;
        seg.build();
        runs = n;
        if (runs <= sparse_low) freeze_dense();
    }
    sortable_segtree(sortable_segtree<T, S, op, e, commutative> &&o) = default;
    sortable_segtree& operator=(sortable_segtree<T, S, op, e, commutative> &&o) = default;
    
    inline S get_root_prod(const Trie &tr) const {
        if constexpr(commutative) return tr.template all_prod_as<proj_ltor, e>();
        else return tr.is_rev() ? tr.template all_prod_as<proj_rtol, e>() : tr.template all_prod_as<proj_ltor, e>();
    }
    inline S query_prefix(const Trie &tr, int len) const {
        if(!tr.is_rev()) return tr.template prod_l_as<S, proj_ltor, op, e>(len);
        else return tr.template prod_r_as<S, proj_rtol, op_rev, e>(len);
    }
    inline S query_suffix(const Trie &tr, int len) const {
        if(!tr.is_rev()) return tr.template prod_r_as<S, proj_ltor, op, e>(len);
        else return tr.template prod_l_as<S, proj_rtol, op_rev, e>(len);
    }
    inline S query_mid(const Trie &tr, int l, int r, int sz) const {
        if(!tr.is_rev()) return tr.template prod_lr_as<S, proj_ltor, op, e>(l, r);
        else return tr.template prod_lr_as<S, proj_rtol, op_rev, e>(sz - r, sz - l);
    }
    
    void freeze_dense() {
        sparse = true;
        frozen_starts.clear();
        for (int b = 0; b < n; b += trie[b].size()) frozen_starts.push_back(seg.siz + b);
    }
    void finish_edit() {
        if (!sparse) {
            seg.propagate_sorted_leaves(buf);
            if (runs <= sparse_low) freeze_dense();
        } else if (runs >= sparse_high) {
            // All former non-identity leaves lie in frozen_starts. Descendants
            // were not overwritten while frozen; restore only old/new starts.
            buf = frozen_starts;
            for (int x : frozen_starts) seg.node[x] = e();
            for (int b = 0; b < n; b += trie[b].size()) {
                seg.node[seg.siz+b] = get_root_prod(trie[b]);
                buf.push_back(seg.siz+b);
            }
            std::sort(buf.begin(), buf.end());
            seg.propagate_sorted_leaves(buf);
            sparse = false;
        }
    }
    S sparse_prod(int l, int r) const {
        int b = fset.less_bound(l), len = trie[b].size(), end = b + len;
        if (r <= end) {
            if (b == l) return r == end ? get_root_prod(trie[b]) : query_prefix(trie[b], r-l);
            return r == end ? query_suffix(trie[b], r-l) : query_mid(trie[b], l-b, r-b, len);
        }
        S acc = b == l ? get_root_prod(trie[b]) : query_suffix(trie[b], end-l);
        do {
            b = end;
            end += trie[b].size();
            acc = op(acc, end <= r ? get_root_prod(trie[b]) : query_prefix(trie[b], r-b));
        } while (end < r);
        return acc;
    }

    void set(int p, T key, S val) {
        int l = fset.less_bound(p);
        int sz = trie[l].size();
        int r = l + sz;
        if(sz == 1){
            trie[p].init(key, val);
            if (!sparse) seg.set(p, val);
            return;
        }
        auto &pool = Trie::get_pool();
        bool old_rev = trie[l].is_rev();
        uint32_t old = trie[l].take_root();
        int pos = old_rev ? (sz - 1 - (p - l)) : (p - l);
        auto dr = Trie::split_drop_at_raw(pool, old, pos);
        uint32_t left_rt = old_rev ? dr.right : dr.left;
        uint32_t right_rt = old_rev ? dr.left : dr.right;
        pool.leaves[Trie::leaf_idx(dr.dropped)] = typename Trie::leaf_t(key, val);
        buf.clear();
        if(l < p){
            trie[l].set_root(left_rt, p - l, old_rev);
            fset.insert(p);
            ++runs;
            if (!sparse) {
                seg.node[l + seg.siz] = get_root_prod(trie[l]);
                buf.push_back(l + seg.siz);
            }
        }
        trie[p].set_root(dr.dropped, 1, false);
        if (!sparse) {
            seg.node[p + seg.siz] = val;
            buf.push_back(p + seg.siz);
        }
        if(p + 1 < r){
            trie[p + 1].set_root(right_rt, r - p - 1, old_rev);
            fset.insert(p + 1);
            ++runs;
            if (!sparse) {
                seg.node[p + 1 + seg.siz] = get_root_prod(trie[p + 1]);
                buf.push_back(p + 1 + seg.siz);
            }
        }
        finish_edit();
    }
    
    S prod(int l, int r) {
        if(l == r) return e();
        if (sparse) return sparse_prod(l, r);
        bool cl = fset.count(l), cr = fset.count(r);
        if(cl && cr) {
            if(l + trie[l].size() == r) return seg.get(l);
            return seg.prod(l, r);
        }
        int lbl = cl ? l : fset.less_bound(l);
        int sz = trie[lbl].size();
        int lbr = lbl + sz;
        if(r <= lbr){
            if(lbl == l){
                if(r == lbr) return seg.get(l);
                return query_prefix(trie[lbl], r - l);
            } else {
                if(r == lbr) return query_suffix(trie[lbl], r - l);
                return query_mid(trie[lbl], l - lbl, r - lbl, sz);
            }
        }
        if(cr){
            S left = query_suffix(trie[lbl], lbr - l);
            if(lbr + trie[lbr].size() == r) return op(left, seg.get(lbr));
            return op(left, seg.prod(lbr, r));
        }
        int rbl = fset.less_bound(r-1);
        if(lbl == l){
            S mid = (lbr == rbl) ? seg.get(l) : seg.prod(l, rbl);
            S right = query_prefix(trie[rbl], r - rbl);
            return op(mid, right);
        }
        S left = query_suffix(trie[lbl], lbr - l);
        if(lbr < rbl) {
            if(lbr + trie[lbr].size() == rbl) left = op(left, seg.get(lbr));
            else left = op(left, seg.prod(lbr, rbl));
        }
        S right = query_prefix(trie[rbl], r - rbl);
        return op(left, right);
    }
    
    static inline pair<uint32_t, uint32_t> split_trie_raw(typename Trie::Pool &pool, Trie &tr, int k) {
        bool r = tr.is_rev();
        auto [old, sz] = tr.take_root_sz();
        int pos = r ? (sz - k) : k;
        auto res = Trie::split(pool, old, sz, pos);
        if(r) return {res.second, res.first};
        return res;
    }

    void _sort(int l, int r, bool is_rev) {
        if(r - l <= 1) return;
        int lb = fset.less_bound(l);
        int lbsz = trie[lb].size();
        int lbr = lb + lbsz;
        auto &pool = Trie::get_pool();
        if(r <= lbr){
            bool old_rev = trie[lb].is_rev();
            if(old_rev == is_rev) return;
            if(lb == l && r == lbr){
                trie[l].set_rev(is_rev);
                if constexpr (!commutative) {
                    if (!sparse) seg.set(l, get_root_prod(trie[l]));
                }
                return;
            }
            buf.clear();
            if(lb < l && r < lbr){
                auto [old, sz] = trie[lb].take_root_sz();
                int a = l - lb, b = r - lb;
                auto sp3 = old_rev
                    ? Trie::split3(pool, old, sz, sz - b, sz - a)
                    : Trie::split3(pool, old, sz, a, b);
                uint32_t lt = old_rev ? sp3.right : sp3.left;
                uint32_t mt = sp3.mid;
                uint32_t rt = old_rev ? sp3.left : sp3.right;
                trie[lb].set_root(lt, a, old_rev);
                trie[l].set_root(mt, r - l, is_rev);
                trie[r].set_root(rt, lbr - r, old_rev);
                fset.insert(l); fset.insert(r);
                runs += 2;
                if (!sparse) {
                    seg.node[lb + seg.siz] = get_root_prod(trie[lb]);
                    seg.node[l + seg.siz] = get_root_prod(trie[l]);
                    seg.node[r + seg.siz] = get_root_prod(trie[r]);
                    buf.push_back(lb + seg.siz);
                    buf.push_back(l + seg.siz);
                    buf.push_back(r + seg.siz);
                }
                finish_edit();
                return;
            }
            if(lb < l){
                auto [lt, rt] = split_trie_raw(pool, trie[lb], l - lb);
                trie[lb].set_root(lt, l - lb, old_rev);
                trie[l].set_root(rt, lbr - l, old_rev);
                fset.insert(l);
                ++runs;
                if (!sparse) {
                    seg.node[lb + seg.siz] = get_root_prod(trie[lb]);
                    buf.push_back(lb + seg.siz);
                }
            }
            if (!sparse) buf.push_back(l + seg.siz);
            if(r < lbr){
                auto [lt, rt] = split_trie_raw(pool, trie[l], r - l);
                trie[l].set_root(lt, r - l, old_rev);
                trie[r].set_root(rt, lbr - r, old_rev);
                fset.insert(r);
                ++runs;
                if (!sparse) {
                    seg.node[r + seg.siz] = get_root_prod(trie[r]);
                    buf.push_back(r + seg.siz);
                }
            }
            trie[l].set_rev(is_rev);
            if (!sparse) seg.node[l + seg.siz] = get_root_prod(trie[l]);
            finish_edit();
            return;
        }
        buf.clear();
        merge_roots.clear();
        if(lb < l){
            bool old_rev = trie[lb].is_rev();
            auto [lt, rt] = split_trie_raw(pool, trie[lb], l - lb);
            trie[lb].set_root(lt, l - lb, old_rev);
            merge_roots.push_back({rt, lbr - l});
            fset.insert(l);
            ++runs;
            if (!sparse) {
                seg.node[lb + seg.siz] = get_root_prod(trie[lb]);
                buf.push_back(lb + seg.siz);
            }
        } else {
            auto [rt, rsz] = trie[l].take_root_sz();
            merge_roots.push_back({rt, rsz});
        }
        int rb = fset.less_bound(r);
        if(rb < r){
            bool old_rev = trie[rb].is_rev();
            int rbsz = trie[rb].size();
            auto [lt, rt] = split_trie_raw(pool, trie[rb], r - rb);
            trie[rb].set_root(lt, r - rb, old_rev);
            trie[r].set_root(rt, rb + rbsz - r, old_rev);
            fset.insert(r);
            ++runs;
            if (!sparse) seg.node[r + seg.siz] = get_root_prod(trie[r]);
        }
        if (!sparse) buf.push_back(l + seg.siz);
        fset.for_each_and_erase(l + 1, r, [&](int b){
            auto [rt, rsz] = trie[b].take_root_sz();
            merge_roots.push_back({rt, rsz});
            if (!sparse) {
                seg.node[b + seg.siz] = e();
                buf.push_back(b + seg.siz);
            }
        });
        uint32_t cur;
        int cur_sz = r - l;
        int k = static_cast<int>(merge_roots.size());
        runs -= k - 1;
        if(k == 1){
            cur = merge_roots[0].rt;
        } else if(k == 2){
            cur = Trie::meld(pool, merge_roots[0].rt, merge_roots[0].sz, merge_roots[1].rt, merge_roots[1].sz);
        } else {
            int leaf_cnt = 0;
            for(const auto &it : merge_roots) if(it.sz == 1) ++leaf_cnt;
            if(leaf_cnt >= 16){
                auto *lf = pool.leaves.data();
                leaf_buf.clear();
                int rem = 0;
                for(const auto &it : merge_roots){
                    if(it.sz == 1) leaf_buf.push_back({lf[Trie::leaf_idx(it.rt)].value, it.rt});
                    else merge_roots[rem++] = it;
                }
                merge_roots.resize(rem);
                int bsz = static_cast<int>(leaf_buf.size());
                merge_roots.push_back({Trie::build_from_leaves(pool, leaf_buf, leaf_tmp), bsz});
                k = static_cast<int>(merge_roots.size());
            }
            if(k == 1){
                cur = merge_roots[0].rt;
            } else if(k == 2){
                cur = Trie::meld(pool, merge_roots[0].rt, merge_roots[0].sz, merge_roots[1].rt, merge_roots[1].sz);
            } else {
                mbuf.clear();
                if(static_cast<int>(mbuf.capacity()) < k * 4) mbuf.reserve(static_cast<size_t>(k) * 4);
                auto *in = pool.inodes.data();
                auto *lf = pool.leaves.data();
                for(const auto &it : merge_roots){
                    mbuf.push_back(Trie::make_mnode(in, lf, it.rt, it.sz));
                }
                cur = Trie::meld_multi(pool, mbuf, 0, k, k);
            }
        }
        if(!sparse && rb < r) buf.push_back(r + seg.siz);
        trie[l].set_root(cur, cur_sz, is_rev);
        if (!sparse) seg.node[l + seg.siz] = get_root_prod(trie[l]);
        finish_edit();
    }
    
    void sort(int l, int r) { _sort(l, r, false); }
    
    void sort_desc(int l, int r) { _sort(l, r, true); }
};
