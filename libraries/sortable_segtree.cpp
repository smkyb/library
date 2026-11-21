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
        // bits [0..6]: split height (1..bit_width), bits [7..31]: subtree count
        uint32_t ch;
        inode_t() = default;
        inode_t(T v, const S &s, uint32_t h, uint32_t c, uint32_t c0, uint32_t c1)
            : sum(s), value(v), child{c0, c1}, ch((c << 7) | h) {}
        inline uint32_t get_h() const noexcept { return ch & 127u; }
        inline int get_cnt() const noexcept { return static_cast<int>(ch >> 7); }
        inline void add_cnt(int d) noexcept { ch += static_cast<uint32_t>(d) << 7; }
        inline void dec_cnt() noexcept { ch -= 128u; }
        inline void set_cnt(int c) noexcept { ch = (ch & 127u) | (static_cast<uint32_t>(c) << 7); }
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
        uint32_t make_inode_slow(T v, const S &s, uint32_t h, uint32_t c, uint32_t c0, uint32_t c1) {
            uint32_t id = static_cast<uint32_t>(inodes.size());
            inodes.emplace_back(v, s, h, c, c0, c1);
            return id;
        }
        inline uint32_t make_inode(T v, const S &s, uint32_t h, uint32_t c, uint32_t c0, uint32_t c1) {
            if(ifree) [[likely]] {
                uint32_t id = ifree;
                ifree = inodes[id].child[0];
                inodes[id] = inode_t(v, s, h, c, c0, c1);
                return id;
            }
            if(inodes.size() == inodes.capacity()) [[unlikely]] return make_inode_slow(v, s, h, c, c0, c1);
            uint32_t id = static_cast<uint32_t>(inodes.size());
            inodes.emplace_back(v, s, h, c, c0, c1);
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
    bool rev = false;
    
    template<typename _Tp>
    inline static uint32_t msb_bit(_Tp x) {
        if constexpr(sizeof(_Tp) == 8ull) return 63u - static_cast<uint32_t>(__builtin_clzll(x));
        else return 31u - static_cast<uint32_t>(__builtin_clz(x));
    }
    
    static inline int node_cnt(const inode_t *in, uint32_t id) noexcept {
        return is_leaf(id) ? 1 : in[id].get_cnt();
    }
    static inline S node_sum(const inode_t *in, const leaf_t *lf, uint32_t id) noexcept {
        return is_leaf(id) ? lift(lf[leaf_idx(id)].val) : in[id].sum;
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
    
    static void reserve(size_t n) {
        get_pool().reserve(n);
    }
    
    meldable_binary_trie() = default;
    meldable_binary_trie(T p, const LeafS &x) : root(get_pool().make_leaf(p, x)) {}
    meldable_binary_trie(uint32_t x) : root(x) {}
    
    meldable_binary_trie(const meldable_binary_trie&) = delete;
    meldable_binary_trie& operator=(const meldable_binary_trie&) = delete;
    
    meldable_binary_trie(meldable_binary_trie&& o) noexcept : root(o.root), rev(o.rev) {
        o.root = NIL;
        o.rev = false;
    }
    meldable_binary_trie& operator=(meldable_binary_trie&& o) noexcept {
        if(this != &o) {
            clear();
            root = o.root;
            rev = o.rev;
            o.root = NIL;
            o.rev = false;
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
        rev = false;
    }
    
    uint32_t take_root() {
        uint32_t r = root;
        root = NIL;
        rev = false;
        return r;
    }
    void set_root(uint32_t r, bool is_rev) {
        root = r;
        rev = (r != NIL) && is_rev;
    }
    
    int size() const {
        if(root == NIL) return 0;
        return is_leaf(root) ? 1 : get_pool().inodes[root].get_cnt();
    }
    
    static uint32_t meld(meldable_binary_trie &l, meldable_binary_trie &r){
        uint32_t lr = l.take_root(), rr = r.take_root();
        return meld(get_pool(), lr, rr);
    }
    static uint32_t meld(uint32_t l, uint32_t r){
        return meld(get_pool(), l, r);
    }
    static uint32_t meld(Pool &pool, uint32_t l, uint32_t r){
        if(l == NIL) return r;
        if(r == NIL) return l;
        uint32_t st[65];
        uint8_t sdir[65];
        int sp = 0;
        uint32_t res;
        while(true){
            auto *in = pool.inodes.data();
            auto *lf = pool.leaves.data();
            uint32_t lh = is_leaf(l) ? 0u : in[l].get_h();
            uint32_t rh = is_leaf(r) ? 0u : in[r].get_h();
            if(lh < rh){ swap(l, r); swap(lh, rh); }
            T lv = (lh == 0u) ? lf[leaf_idx(l)].value : in[l].value;
            T rv = (rh == 0u) ? lf[leaf_idx(r)].value : in[r].value;
            T x = lv ^ rv;
            if(x != 0){
                uint32_t b = msb_bit(x);
                if(b >= lh){
                    uint32_t dir = (rv >> b) & 1u;
                    uint32_t c0 = dir ? l : r;
                    uint32_t c1 = dir ? r : l;
                    S s = op(node_sum(in, lf, c0), node_sum(in, lf, c1));
                    int cnt = node_cnt(in, c0) + node_cnt(in, c1);
                    res = pool.make_inode(lv, s, b + 1u, static_cast<uint32_t>(cnt), c0, c1);
                    break;
                }
            }
            if(lh == rh){
                in[l].add_cnt(in[r].get_cnt());
                uint32_t lc0 = in[l].child[0], lc1 = in[l].child[1];
                uint32_t rc0 = in[r].child[0], rc1 = in[r].child[1];
                pool.push_inode(r);
                uint32_t nc0 = meld(pool, lc0, rc0);
                uint32_t nc1 = meld(pool, lc1, rc1);
                in = pool.inodes.data();
                lf = pool.leaves.data();
                in[l].child[0] = nc0;
                in[l].child[1] = nc1;
                in[l].sum = op(node_sum(in, lf, nc0), node_sum(in, lf, nc1));
                res = l;
                break;
            }
            int rcnt = (rh == 0u) ? 1 : in[r].get_cnt();
            in[l].add_cnt(rcnt);
            uint32_t dir = (rv >> (lh - 1u)) & 1u;
            st[sp] = l;
            sdir[sp] = static_cast<uint8_t>(dir);
            ++sp;
            l = in[l].child[dir];
        }
        auto *in = pool.inodes.data();
        auto *lf = pool.leaves.data();
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
            if(m >= 256){
                tmp.resize(m);
                uint32_t cnt[2048];
                memset(cnt, 0, sizeof(uint32_t) * 2048);
                for(int i = 0; i < m; i++) cnt[items[i].key & 2047u]++;
                uint32_t acc = 0;
                for(int i = 0; i < 2048; i++){ uint32_t c = cnt[i]; cnt[i] = acc; acc += c; }
                for(int i = 0; i < m; i++) tmp[cnt[items[i].key & 2047u]++] = items[i];
                
                memset(cnt, 0, sizeof(uint32_t) * 2048);
                for(int i = 0; i < m; i++) cnt[(tmp[i].key >> 11) & 2047u]++;
                acc = 0;
                for(int i = 0; i < 2048; i++){ uint32_t c = cnt[i]; cnt[i] = acc; acc += c; }
                for(int i = 0; i < m; i++) items[cnt[(tmp[i].key >> 11) & 2047u]++] = tmp[i];
                
                memset(cnt, 0, sizeof(uint32_t) * 1024);
                for(int i = 0; i < m; i++) cnt[items[i].key >> 22]++;
                acc = 0;
                for(int i = 0; i < 1024; i++){ uint32_t c = cnt[i]; cnt[i] = acc; acc += c; }
                for(int i = 0; i < m; i++) tmp[cnt[items[i].key >> 22]++] = items[i];
                items.swap(tmp);
            } else {
                std::sort(items.begin(), items.end(), [](const LeafItem &a, const LeafItem &b){ return a.key < b.key; });
            }
        } else {
            std::sort(items.begin(), items.end(), [](const LeafItem &a, const LeafItem &b){ return a.key < b.key; });
        }
        uint32_t st_node[65], st_h[65];
        T st_key[65];
        int sp = 0;
        st_node[0] = items[0].id;
        st_key[0] = items[0].key;
        st_h[0] = 0;
        sp = 1;
        for(int i = 1; i < m; ++i){
            uint32_t bh = msb_bit(items[i - 1].key ^ items[i].key) + 1u;
            while(sp >= 2 && st_h[sp - 1] < bh){
                uint32_t rnode = st_node[sp - 1], lnode = st_node[sp - 2], ph = st_h[sp - 1];
                auto *in = pool.inodes.data();
                auto *lf = pool.leaves.data();
                S s = op(node_sum(in, lf, lnode), node_sum(in, lf, rnode));
                uint32_t p = pool.make_inode(st_key[sp - 2], s, ph,
                                             static_cast<uint32_t>(node_cnt(in, lnode) + node_cnt(in, rnode)),
                                             lnode, rnode);
                --sp;
                st_node[sp - 1] = p;
            }
            st_node[sp] = items[i].id;
            st_key[sp] = items[i].key;
            st_h[sp] = bh;
            ++sp;
        }
        while(sp >= 2){
            uint32_t rnode = st_node[sp - 1], lnode = st_node[sp - 2], ph = st_h[sp - 1];
            auto *in = pool.inodes.data();
            auto *lf = pool.leaves.data();
            S s = op(node_sum(in, lf, lnode), node_sum(in, lf, rnode));
            uint32_t p = pool.make_inode(st_key[sp - 2], s, ph,
                                         static_cast<uint32_t>(node_cnt(in, lnode) + node_cnt(in, rnode)),
                                         lnode, rnode);
            --sp;
            st_node[sp - 1] = p;
        }
        return st_node[0];
    }
    
    pair<meldable_binary_trie, meldable_binary_trie> split(int p){
        auto &pool = get_pool();
        bool old_rev = rev;
        uint32_t old = take_root();
        int sz = (old == NIL) ? 0 : node_cnt(pool.inodes.data(), old);
        if(old_rev) p = sz - p;
        auto res = split(pool, old, p);
        if(old_rev){
            meldable_binary_trie f(res.second), s(res.first);
            if(f.root != NIL) f.rev = true;
            if(s.root != NIL) s.rev = true;
            return make_pair(move(f), move(s));
        } else return {meldable_binary_trie(res.first), meldable_binary_trie(res.second)};
    }
    
    static pair<uint32_t, uint32_t> split(uint32_t l, int p){
        return split(get_pool(), l, p);
    }
    static pair<uint32_t, uint32_t> split(Pool &pool, uint32_t l, int p){
        auto *in = pool.inodes.data();
        auto *lf = pool.leaves.data();
        if(p == 0) return {NIL, l};
        if(p == node_cnt(in, l)) return {l, NIL};
        struct Frame { uint32_t node; uint8_t dir; };
        Frame st[65];
        int sp = 0;
        uint32_t rl = NIL, rr = NIL;
        while(true){
            uint32_t c0 = in[l].child[0], c1 = in[l].child[1];
            int c0_cnt = node_cnt(in, c0);
            if(p == c0_cnt){
                pool.push_inode(l);
                rl = c0; rr = c1;
                break;
            } else if(p < c0_cnt){
                st[sp++] = {l, 0};
                l = c0;
            } else {
                st[sp++] = {l, 1};
                p -= c0_cnt;
                l = c1;
            }
        }
        for(int i = sp - 1; i >= 0; --i){
            uint32_t node = st[i].node;
            if(st[i].dir == 0){
                in[node].child[0] = rr;
                uint32_t c1 = in[node].child[1];
                in[node].set_cnt(node_cnt(in, rr) + node_cnt(in, c1));
                in[node].sum = op(node_sum(in, lf, rr), node_sum(in, lf, c1));
                rr = node;
            } else {
                in[node].child[1] = rl;
                uint32_t c0 = in[node].child[0];
                in[node].set_cnt(node_cnt(in, c0) + node_cnt(in, rl));
                in[node].sum = op(node_sum(in, lf, c0), node_sum(in, lf, rl));
                rl = node;
            }
        }
        return {rl, rr};
    }
    
    static DropRes split_drop_at_raw(Pool &pool, uint32_t l, int p){
        auto *in = pool.inodes.data();
        auto *lf = pool.leaves.data();
        if(is_leaf(l)) return {NIL, l, NIL};
        struct Frame { uint32_t node; uint8_t dir; };
        Frame st[65];
        int sp = 0;
        while(!is_leaf(l)){
            uint32_t c0 = in[l].child[0];
            int c0_cnt = node_cnt(in, c0);
            if(p < c0_cnt){
                st[sp++] = {l, 0};
                l = c0;
            } else {
                st[sp++] = {l, 1};
                p -= c0_cnt;
                l = in[l].child[1];
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
                    in[node].set_cnt(node_cnt(in, rr) + node_cnt(in, c1));
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
                    in[node].set_cnt(node_cnt(in, c0) + node_cnt(in, rl));
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
        bool old_rev = rev;
        uint32_t old = take_root();
        if(old_rev){
            meldable_binary_trie res(split_one_dir<1>(pool, old));
            if(res.root != NIL) res.rev = true;
            return res;
        } else {
            return meldable_binary_trie(split_one_dir<0>(pool, old));
        }
    }
    template<int DIR>
    static uint32_t split_one_dir(Pool &pool, uint32_t l){
        auto *in = pool.inodes.data();
        auto *lf = pool.leaves.data();
        if(is_leaf(l)){
            pool.push_leaf(l);
            return NIL;
        }
        uint32_t st[65];
        int sp = 0;
        while(!is_leaf(l)){
            in[l].dec_cnt();
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
    static R _prod_l_proj(int n, uint32_t pos, const inode_t *in, const leaf_t *lf) {
        if(n == 0 || pos == NIL) return sub_e();
        if(n == node_cnt(in, pos)) return node_proj<R, proj, leaf_proj>(in, lf, pos);
        R res = sub_e();
        bool has = false;
        while(true) {
            uint32_t c0p = in[pos].child[0];
            int c0 = node_cnt(in, c0p);
            if(n < c0) {
                pos = c0p;
            } else if(n == c0) {
                R v = node_proj<R, proj, leaf_proj>(in, lf, c0p);
                return has ? sub_op(res, v) : v;
            } else {
                R v = node_proj<R, proj, leaf_proj>(in, lf, c0p);
                res = has ? sub_op(res, v) : v;
                has = true;
                n -= c0;
                pos = in[pos].child[1];
                if(n == node_cnt(in, pos)) return sub_op(res, node_proj<R, proj, leaf_proj>(in, lf, pos));
            }
        }
    }
    
    template<typename R, auto proj, auto leaf_proj, auto sub_op, auto sub_e>
    static R _prod_r_proj(int n, uint32_t pos, const inode_t *in, const leaf_t *lf) {
        if(n == 0 || pos == NIL) return sub_e();
        if(n == node_cnt(in, pos)) return node_proj<R, proj, leaf_proj>(in, lf, pos);
        R res = sub_e();
        bool has = false;
        while(true) {
            uint32_t c1p = in[pos].child[1];
            int c1 = node_cnt(in, c1p);
            if(n < c1) {
                pos = c1p;
            } else if(n == c1) {
                R v = node_proj<R, proj, leaf_proj>(in, lf, c1p);
                return has ? sub_op(v, res) : v;
            } else {
                R v = node_proj<R, proj, leaf_proj>(in, lf, c1p);
                res = has ? sub_op(v, res) : v;
                has = true;
                n -= c1;
                pos = in[pos].child[0];
                if(n == node_cnt(in, pos)) return sub_op(node_proj<R, proj, leaf_proj>(in, lf, pos), res);
            }
        }
    }
    
    template<typename R, auto proj, auto leaf_proj, auto sub_op, auto sub_e>
    static R _prod_lr_proj(int l, int r, uint32_t pos, const inode_t *in, const leaf_t *lf) {
        if(l == r || pos == NIL) return sub_e();
        while(true) {
            if(l == 0) return _prod_l_proj<R, proj, leaf_proj, sub_op, sub_e>(r, pos, in, lf);
            if(r == node_cnt(in, pos)) return _prod_r_proj<R, proj, leaf_proj, sub_op, sub_e>(r - l, pos, in, lf);
            uint32_t c0p = in[pos].child[0], c1p = in[pos].child[1];
            int c0 = node_cnt(in, c0p);
            if(l == c0) return _prod_l_proj<R, proj, leaf_proj, sub_op, sub_e>(r - l, c1p, in, lf);
            if(r == c0) return _prod_r_proj<R, proj, leaf_proj, sub_op, sub_e>(r - l, c0p, in, lf);
            if(l < c0) {
                if(r < c0) pos = c0p;
                else return sub_op(_prod_r_proj<R, proj, leaf_proj, sub_op, sub_e>(c0 - l, c0p, in, lf),
                                   _prod_l_proj<R, proj, leaf_proj, sub_op, sub_e>(r - c0, c1p, in, lf));
            } else {
                l -= c0;
                r -= c0;
                pos = c1p;
            }
        }
    }
    
    static inline const S& _id_sum(const S &x) noexcept { return x; }
    static inline const LeafS& _id_leaf(const LeafS &x) noexcept { return x; }
    
    S prod_l(int n) const {
        const auto &p = get_pool();
        return _prod_l_proj<S, _id_sum, lift, op, e>(n, root, p.inodes.data(), p.leaves.data());
    }
    S prod_r(int n) const {
        const auto &p = get_pool();
        return _prod_r_proj<S, _id_sum, lift, op, e>(n, root, p.inodes.data(), p.leaves.data());
    }
    S prod_lr(int l, int r) const {
        const auto &p = get_pool();
        return _prod_lr_proj<S, _id_sum, lift, op, e>(l, r, root, p.inodes.data(), p.leaves.data());
    }
    
    template<typename R, auto proj, auto sub_op, auto sub_e>
    R prod_l_as(int n) const {
        const auto &p = get_pool();
        return _prod_l_proj<R, proj, _id_leaf, sub_op, sub_e>(n, root, p.inodes.data(), p.leaves.data());
    }
    template<typename R, auto proj, auto sub_op, auto sub_e>
    R prod_r_as(int n) const {
        const auto &p = get_pool();
        return _prod_r_proj<R, proj, _id_leaf, sub_op, sub_e>(n, root, p.inodes.data(), p.leaves.data());
    }
    template<typename R, auto proj, auto sub_op, auto sub_e>
    R prod_lr_as(int l, int r) const {
        const auto &p = get_pool();
        return _prod_lr_proj<R, proj, _id_leaf, sub_op, sub_e>(l, r, root, p.inodes.data(), p.leaves.data());
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
        rev = false;
        if(root != NIL){
            if(is_leaf(root)){
                get_pool().leaves[leaf_idx(root)] = leaf_t(p, x);
                return;
            }
            clear();
        }
        root = get_pool().make_leaf(p, x);
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
    
    void propagate_sorted_leaves(vector<int> &buf){
        while(buf.size() > 1 || buf[0] > 1){
            int m = 0;
            for(int x : buf){
                int p = x >> 1;
                if(m == 0 || buf[m-1] != p) buf[m++] = p;
            }
            buf.resize(m);
            for(int i = 0; i < m; i++){
                int p = buf[i];
                node[p] = op(node[p<<1], node[(p<<1)|1]);
            }
        }
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
    
    int n;
    vector<Trie> trie;
    smkyb::fastset fset;
    smkyb::segtree<S, op, e> seg;
    vector<int> buf;
    vector<uint32_t> merge_roots;
    vector<typename Trie::LeafItem> leaf_buf, leaf_tmp;
    
    sortable_segtree(const vector<pair<T, S>> &v) : n(v.size()), trie(n), fset(n+1, true), seg(n) {
        Trie::reserve(static_cast<size_t>(n));
        auto &pool = Trie::get_pool();
        for(int i = 0; i < n; i++) trie[i].set_root(pool.make_leaf(v[i].first, v[i].second), false);
        for(int i = 0; i < n; i++) seg.node[i + seg.siz] = v[i].second;
        seg.build();
    }
    sortable_segtree(sortable_segtree<T, S, op, e, commutative> &&o) = default;
    sortable_segtree& operator=(sortable_segtree<T, S, op, e, commutative> &&o) = default;
    
    inline S get_root_prod(const Trie &tr) const {
        if constexpr(commutative) return tr.template all_prod_as<proj_ltor, e>();
        else return tr.rev ? tr.template all_prod_as<proj_rtol, e>() : tr.template all_prod_as<proj_ltor, e>();
    }
    inline S query_prefix(const Trie &tr, int len) const {
        if(!tr.rev) return tr.template prod_l_as<S, proj_ltor, op, e>(len);
        else return tr.template prod_r_as<S, proj_rtol, op_rev, e>(len);
    }
    inline S query_suffix(const Trie &tr, int len) const {
        if(!tr.rev) return tr.template prod_r_as<S, proj_ltor, op, e>(len);
        else return tr.template prod_l_as<S, proj_rtol, op_rev, e>(len);
    }
    inline S query_mid(const Trie &tr, int l, int r, int sz) const {
        if(!tr.rev) return tr.template prod_lr_as<S, proj_ltor, op, e>(l, r);
        else return tr.template prod_lr_as<S, proj_rtol, op_rev, e>(sz - r, sz - l);
    }
    
    void set(int p, T key, S val) {
        int l = fset.less_bound(p);
        int sz = trie[l].size();
        int r = l + sz;
        if(sz == 1){
            trie[p].init(key, val);
            seg.set(p, val);
            return;
        }
        auto &pool = Trie::get_pool();
        bool old_rev = trie[l].rev;
        uint32_t old = trie[l].take_root();
        int pos = old_rev ? (sz - 1 - (p - l)) : (p - l);
        auto dr = Trie::split_drop_at_raw(pool, old, pos);
        uint32_t left_rt = old_rev ? dr.right : dr.left;
        uint32_t right_rt = old_rev ? dr.left : dr.right;
        pool.leaves[Trie::leaf_idx(dr.dropped)] = typename Trie::leaf_t(key, val);
        buf.clear();
        if(l < p){
            trie[l].set_root(left_rt, old_rev);
            fset.insert(p);
            seg.node[l + seg.siz] = get_root_prod(trie[l]);
            buf.push_back(l + seg.siz);
        }
        trie[p].set_root(dr.dropped, false);
        seg.node[p + seg.siz] = val;
        buf.push_back(p + seg.siz);
        if(p + 1 < r){
            trie[p + 1].set_root(right_rt, old_rev);
            fset.insert(p + 1);
            seg.node[p + 1 + seg.siz] = get_root_prod(trie[p + 1]);
            buf.push_back(p + 1 + seg.siz);
        }
        seg.propagate_sorted_leaves(buf);
    }
    
    S prod(int l, int r) {
        if(l == r) return e();
        bool cl = fset.count(l), cr = fset.count(r);
        if(cl && cr) return seg.prod(l, r);
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
            return op(left, seg.prod(lbr, r));
        }
        int rbl = fset.less_bound(r-1);
        if(lbl == l){
            S mid = seg.prod(l, rbl);
            S right = query_prefix(trie[rbl], r - rbl);
            return op(mid, right);
        }
        S left = query_suffix(trie[lbl], lbr - l);
        if(lbr < rbl) left = op(left, seg.prod(lbr, rbl));
        S right = query_prefix(trie[rbl], r - rbl);
        return op(left, right);
    }
    
    void _sort(int l, int r, bool is_rev) {
        if(r - l <= 1) return;
        int lb = fset.less_bound(l);
        int lbr = lb + trie[lb].size();
        if(r <= lbr){
            if(trie[lb].rev == is_rev) return;
            if(lb == l && r == lbr){
                trie[l].rev = is_rev;
                seg.set(l, get_root_prod(trie[l]));
                return;
            }
            buf.clear();
            if(lb < l){
                auto res = trie[lb].split(l-lb);
                trie[lb] = move(res.first); trie[l] = move(res.second);
                fset.insert(l);
                seg.node[lb + seg.siz] = get_root_prod(trie[lb]);
                buf.push_back(lb + seg.siz);
            }
            buf.push_back(l + seg.siz);
            if(r < lbr){
                auto res = trie[l].split(r-l);
                trie[l] = move(res.first); trie[r] = move(res.second);
                fset.insert(r);
                seg.node[r + seg.siz] = get_root_prod(trie[r]);
                buf.push_back(r + seg.siz);
            }
            trie[l].rev = is_rev;
            seg.node[l + seg.siz] = get_root_prod(trie[l]);
            seg.propagate_sorted_leaves(buf);
            return;
        }
        buf.clear();
        if(lb < l){
            auto res = trie[lb].split(l-lb);
            trie[lb] = move(res.first); trie[l] = move(res.second);
            fset.insert(l);
            seg.node[lb + seg.siz] = get_root_prod(trie[lb]);
            buf.push_back(lb + seg.siz);
        }
        int rb = fset.less_bound(r);
        if(rb < r){
            auto res = trie[rb].split(r-rb);
            trie[rb] = move(res.first); trie[r] = move(res.second);
            fset.insert(r);
            seg.node[r + seg.siz] = get_root_prod(trie[r]);
        }
        auto &pool = Trie::get_pool();
        buf.push_back(l + seg.siz);
        merge_roots.clear();
        merge_roots.push_back(trie[l].take_root());
        fset.for_each_and_erase(l + 1, r, [&](int b){
            merge_roots.push_back(trie[b].take_root());
            seg.node[b + seg.siz] = e();
            buf.push_back(b + seg.siz);
        });
        uint32_t cur;
        int k = static_cast<int>(merge_roots.size());
        if(k == 1){
            cur = merge_roots[0];
        } else if(k == 2){
            cur = Trie::meld(pool, merge_roots[0], merge_roots[1]);
        } else {
            int leaf_cnt = 0;
            for(uint32_t rt : merge_roots) if(Trie::is_leaf(rt)) ++leaf_cnt;
            if(leaf_cnt >= 16){
                auto *lf = pool.leaves.data();
                leaf_buf.clear();
                int rem = 0;
                for(uint32_t rt : merge_roots){
                    if(Trie::is_leaf(rt)) leaf_buf.push_back({lf[Trie::leaf_idx(rt)].value, rt});
                    else merge_roots[rem++] = rt;
                }
                merge_roots.resize(rem);
                merge_roots.push_back(Trie::build_from_leaves(pool, leaf_buf, leaf_tmp));
                k = static_cast<int>(merge_roots.size());
            }
            cur = merge_roots[0];
            for(int i = 1; i < k; ++i) cur = Trie::meld(pool, cur, merge_roots[i]);
        }
        if(rb < r) buf.push_back(r + seg.siz);
        trie[l].set_root(cur, is_rev);
        seg.node[l + seg.siz] = get_root_prod(trie[l]);
        seg.propagate_sorted_leaves(buf);
    }
    
    void sort(int l, int r) { _sort(l, r, false); }
    
    void sort_desc(int l, int r) { _sort(l, r, true); }
};
