// 注意: 符号なし整数型 T (sizeof(T) <= 8) 専用・重複キー可; 要素数 n < 2^25 (33,554,432)
//       コピー不可 (ムーブのみ)・型 T ごとに静的メモリプールを共有; sort/sort_desc は半開 [l, r) (範囲チェックなし)
template<typename T>
struct sortable_array {
    struct fastset{
        using ull = unsigned long long;
        using uint = unsigned;
        
        int siz;
        vector<ull> node;
        
        fastset(int _n, bool fill = false){
            int n = 1;
            while((n<<6) < _n) n <<= 6;
            siz = (n+1)/63;
            node.resize((n+1)/63 + (_n+63)/64);
            if(fill && _n > 0){
                uint cur = (uint)siz;
                uint len = (uint)((_n + 63) >> 6);
                uint rem = (uint)_n & 63u;
                while(true){
                    for(uint i = 0; i + 1 < len; ++i) node[cur + i] = ~0ull;
                    node[cur + len - 1] = rem ? ((1ull << rem) - 1ull) : ~0ull;
                    if(cur == 0) break;
                    cur = (cur - 1) >> 6;
                    rem = len & 63u;
                    len = (len + 63u) >> 6;
                }
            }
        }
        
        void insert(int x){
            uint idx = (uint)siz + ((uint)x>>6);
            uint b = (uint)x & 63u;
            while(true){
                if((node[idx]>>b) & 1ull) return;
                node[idx] |= (1ull<<b);
                if(idx == 0) return;
                idx--;
                b = idx & 63u;
                idx >>= 6;
            }
        }
        
        void erase(int x){
            uint idx = (uint)siz + ((uint)x>>6);
            uint b = (uint)x & 63u;
            while(true){
                node[idx] &= ~(1ull<<b);
                if(idx == 0 || node[idx]) return;
                idx--;
                b = idx & 63u;
                idx >>= 6;
            }
        }
        
        bool count(int x) const {
            return (node[(uint)siz+((uint)x>>6)]>>((uint)x&63u))&1;
        }
        
        int lower_bound(int x) const {
            uint idx = (uint)siz + ((uint)x>>6);
            uint b = (uint)x & 63u;
            ull mask = node[idx] & (~0ull << b);
            if(mask) return (int)((idx - (uint)siz)*64 + __builtin_ctzll(mask));
            while(idx > 0){
                idx--;
                b = idx & 63u;
                idx >>= 6;
                if(b < 63u && (mask = node[idx] & (~1ull << b))){
                    b = __builtin_ctzll(mask);
                    idx = (idx<<6) + b + 1;
                    while(idx < (uint)siz) idx = (idx<<6) + __builtin_ctzll(node[idx]) + 1;
                    return (int)((idx - (uint)siz)*64 + __builtin_ctzll(node[idx]));
                }
            }
            return -1;
        }
        
        int less_bound(int x) const {
            uint idx = (uint)siz + ((uint)x>>6);
            uint b = (uint)x & 63u;
            ull mask = node[idx] & (~0ull >> (63u - b));
            if(mask) return (int)((idx - (uint)siz)*64 + 63 - __builtin_clzll(mask));
            while(idx > 0){
                idx--;
                b = idx & 63u;
                idx >>= 6;
                if(b > 0 && (mask = node[idx] & ((1ull << b) - 1ull))){
                    b = 63 - __builtin_clzll(mask);
                    idx = (idx<<6) + b + 1;
                    while(idx < (uint)siz) idx = (idx<<6) + 64 - __builtin_clzll(node[idx]);
                    return (int)((idx - (uint)siz)*64 + 63 - __builtin_clzll(node[idx]));
                }
            }
            return -1;
        }
        
        template<typename F>
        void for_each_and_erase(int l, int r, F &&fn){
            if(l >= r) return;
            int p = lower_bound(l);
            while(p != -1 && p < r){
                uint w = static_cast<uint>(p) >> 6;
                uint idx = static_cast<uint>(siz) + w;
                uint b0 = static_cast<uint>(p) & 63u;
                ull hi_mask = (static_cast<uint>(r - 1) >> 6 == w)
                    ? (~0ull >> (63u - (static_cast<uint>(r - 1) & 63u)))
                    : ~0ull;
                ull bits = node[idx] & ((~0ull << b0) & hi_mask);
                node[idx] &= ~bits;
                if(node[idx] == 0){
                    uint cur = idx;
                    while(cur > 0){
                        cur--;
                        uint b = cur & 63u;
                        cur >>= 6;
                        node[cur] &= ~(1ull << b);
                        if(node[cur]) break;
                    }
                }
                uint base = w << 6;
                while(bits){
                    int bit = __builtin_ctzll(bits);
                    fn(static_cast<int>(base + static_cast<uint>(bit)));
                    bits &= bits - 1;
                }
                int next_start = static_cast<int>((w + 1) << 6);
                if(next_start >= r) break;
                p = lower_bound(next_start);
            }
        }
    };
    
    struct binary_trie{
        static_assert(is_unsigned_v<T> && sizeof(T) <= 8);
        
        static constexpr int bit_width = sizeof(T) * 8;
        static constexpr uint32_t NIL = 0;
        static constexpr uint32_t LEAF_BIT = 0x80000000u;
        
        static inline bool is_leaf(uint32_t id) noexcept { return (id & LEAF_BIT) != 0; }
        static inline uint32_t leaf_idx(uint32_t id) noexcept { return id & ~LEAF_BIT; }
        static inline uint32_t make_leaf_id(uint32_t idx) noexcept { return idx | LEAF_BIT; }
        
        struct leaf_t{
            T value;
            int count;
            leaf_t() = default;
            leaf_t(T v, int c) : value(v), count(c) {}
        };
        
        static constexpr size_t INODE_ALIGN =
            (sizeof(T) + 12 <= 16 && alignof(T) <= 16) ? 16 : alignof(T);
        
        struct alignas(INODE_ALIGN) inode_t{
            T value;
            uint32_t child[2];
            // bits [0..6]: split height (1..bit_width), bits [7..31]: subtree count
            uint32_t ch;
            inode_t() = default;
            inode_t(T v, uint32_t h, uint32_t c, uint32_t c0, uint32_t c1)
                : value(v), child{c0, c1}, ch((c << 7) | h) {}
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
                inodes.emplace_back(0, 0, 0, NIL, NIL);
                leaves.emplace_back(0, 0);
            }
            ~Pool() {
                pool_alive = false;
            }
            void reserve(size_t n_leaves) {
                if(inodes.capacity() < inodes.size() + n_leaves + 4) inodes.reserve(inodes.size() + n_leaves + 4);
                if(leaves.capacity() < leaves.size() + n_leaves + 4) leaves.reserve(leaves.size() + n_leaves + 4);
            }
            __attribute__((noinline))
            uint32_t make_inode_slow(T v, uint32_t h, uint32_t c, uint32_t c0, uint32_t c1) {
                uint32_t id = static_cast<uint32_t>(inodes.size());
                inodes.emplace_back(v, h, c, c0, c1);
                return id;
            }
            inline uint32_t make_inode(T v, uint32_t h, uint32_t c, uint32_t c0, uint32_t c1) {
                if(ifree) [[likely]] {
                    uint32_t id = ifree;
                    ifree = inodes[id].child[0];
                    inodes[id] = inode_t(v, h, c, c0, c1);
                    return id;
                }
                if(inodes.size() == inodes.capacity()) [[unlikely]] return make_inode_slow(v, h, c, c0, c1);
                uint32_t id = static_cast<uint32_t>(inodes.size());
                inodes.emplace_back(v, h, c, c0, c1);
                return id;
            }
            inline void push_inode(uint32_t id) {
                inodes[id].child[0] = ifree;
                ifree = id;
            }
            __attribute__((noinline))
            uint32_t make_leaf_slow(T v, int c) {
                uint32_t idx = static_cast<uint32_t>(leaves.size());
                leaves.emplace_back(v, c);
                return make_leaf_id(idx);
            }
            inline uint32_t make_leaf(T v, int c) {
                if(lfree) {
                    uint32_t idx = lfree;
                    lfree = static_cast<uint32_t>(leaves[idx].count);
                    leaves[idx] = leaf_t(v, c);
                    return make_leaf_id(idx);
                }
                if(leaves.size() == leaves.capacity()) [[unlikely]] return make_leaf_slow(v, c);
                uint32_t idx = static_cast<uint32_t>(leaves.size());
                leaves.emplace_back(v, c);
                return make_leaf_id(idx);
            }
            inline void push_leaf(uint32_t id) {
                uint32_t idx = leaf_idx(id);
                leaves[idx].count = static_cast<int>(lfree);
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
            else return 31u - static_cast<uint32_t>(__builtin_clz(static_cast<unsigned int>(x)));
        }
        
        static inline int node_cnt(const inode_t *in, const leaf_t *lf, uint32_t id) noexcept {
            return is_leaf(id) ? lf[leaf_idx(id)].count : in[id].get_cnt();
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
        
        struct root_tag {};
        binary_trie() = default;
        binary_trie(T p) : root(get_pool().make_leaf(p, 1)) {}
        binary_trie(root_tag, uint32_t x, bool r = false) : root(x), rev(x != NIL && r) {}
        
        binary_trie(const binary_trie&) = delete;
        binary_trie& operator=(const binary_trie&) = delete;
        
        binary_trie(binary_trie&& o) noexcept : root(o.root), rev(o.rev) {
            o.root = NIL;
            o.rev = false;
        }
        binary_trie& operator=(binary_trie&& o) noexcept {
            if(this != &o) {
                clear();
                root = o.root;
                rev = o.rev;
                o.root = NIL;
                o.rev = false;
            }
            return *this;
        }
        ~binary_trie() {
            clear();
        }
        
        void clear() {
            if(root != NIL) {
                if(pool_alive) _clear(root);
                root = NIL;
            }
            rev = false;
        }
        
        uint32_t peek_root() const noexcept { return root; }
        bool is_single_leaf() const noexcept { return is_leaf(root); }
        
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
        
        T operator[](int k) const {
            const auto &pool = get_pool();
            const auto *in = pool.inodes.data();
            const auto *lf = pool.leaves.data();
            uint32_t pos = root;
            while(!is_leaf(pos)){
                uint32_t c0 = in[pos].child[0];
                int cnt0 = node_cnt(in, lf, c0);
                if(k < cnt0) pos = c0;
                else { k -= cnt0; pos = in[pos].child[1]; }
            }
            return lf[leaf_idx(pos)].value;
        }
        
        int size() const {
            if(root == NIL) return 0;
            const auto &pool = get_pool();
            return node_cnt(pool.inodes.data(), pool.leaves.data(), root);
        }
        
        static uint32_t meld(binary_trie &l, binary_trie &r){
            uint32_t lr = l.take_root(), rr = r.take_root();
            return meld(get_pool(), lr, rr);
        }
        static uint32_t meld(uint32_t l, uint32_t r){
            return meld(get_pool(), l, r);
        }
        static uint32_t meld(Pool &pool, uint32_t l, uint32_t r){
            if(l == NIL) return r;
            if(r == NIL) return l;
            uint32_t top = NIL, parent = NIL, pdir = 0;
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
                        int cnt = node_cnt(in, lf, c0) + node_cnt(in, lf, c1);
                        uint32_t nw = pool.make_inode(lv, b + 1u, static_cast<uint32_t>(cnt), c0, c1);
                        if(parent == NIL) return nw;
                        pool.inodes[parent].child[pdir] = nw;
                        return top;
                    }
                }
                if(lh == rh){
                    if(lh == 0u){
                        lf[leaf_idx(l)].count += lf[leaf_idx(r)].count;
                        pool.push_leaf(r);
                        if(parent == NIL) return l;
                        in[parent].child[pdir] = l;
                        return top;
                    }
                    in[l].add_cnt(in[r].get_cnt());
                    uint32_t lc0 = in[l].child[0], lc1 = in[l].child[1];
                    uint32_t rc0 = in[r].child[0], rc1 = in[r].child[1];
                    pool.push_inode(r);
                    if(parent != NIL) in[parent].child[pdir] = l;
                    else top = l;
                    uint32_t nc0 = meld(pool, lc0, rc0);
                    uint32_t nc1 = meld(pool, lc1, rc1);
                    in = pool.inodes.data();
                    in[l].child[0] = nc0;
                    in[l].child[1] = nc1;
                    return top;
                }
                int rcnt = (rh == 0u) ? lf[leaf_idx(r)].count : in[r].get_cnt();
                in[l].add_cnt(rcnt);
                if(parent != NIL) in[parent].child[pdir] = l;
                else top = l;
                uint32_t dir = (rv >> (lh - 1u)) & 1u;
                parent = l;
                pdir = dir;
                l = in[l].child[dir];
            }
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
            {
                auto *lf = pool.leaves.data();
                int u = 0;
                for(int i = 1; i < m; ++i){
                    if(items[i].key == items[u].key){
                        lf[leaf_idx(items[u].id)].count += lf[leaf_idx(items[i].id)].count;
                        pool.push_leaf(items[i].id);
                    } else {
                        items[++u] = items[i];
                    }
                }
                m = u + 1;
            }
            if(m == 1) return items[0].id;
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
                    uint32_t p = pool.make_inode(st_key[sp - 2], ph,
                                                 static_cast<uint32_t>(node_cnt(in, lf, lnode) + node_cnt(in, lf, rnode)),
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
                uint32_t p = pool.make_inode(st_key[sp - 2], ph,
                                             static_cast<uint32_t>(node_cnt(in, lf, lnode) + node_cnt(in, lf, rnode)),
                                             lnode, rnode);
                --sp;
                st_node[sp - 1] = p;
            }
            return st_node[0];
        }
        
        pair<binary_trie, binary_trie> split(int p){
            auto &pool = get_pool();
            bool old_rev = rev;
            uint32_t old = take_root();
            int sz = (old == NIL) ? 0 : node_cnt(pool.inodes.data(), pool.leaves.data(), old);
            if(old_rev) p = sz - p;
            auto res = split(pool, old, p);
            if(old_rev){
                return {binary_trie(root_tag{}, res.second, true), binary_trie(root_tag{}, res.first, true)};
            } else {
                return {binary_trie(root_tag{}, res.first), binary_trie(root_tag{}, res.second)};
            }
        }
        
        static pair<uint32_t, uint32_t> split(Pool &pool, uint32_t l, int p){
            auto *in = pool.inodes.data();
            auto *lf = pool.leaves.data();
            if(p == 0) return {NIL, l};
            if(p == node_cnt(in, lf, l)) return {l, NIL};
            struct Frame { uint32_t node; uint8_t dir; };
            Frame st[65];
            int sp = 0;
            uint32_t rl = NIL, rr = NIL;
            while(true){
                if(is_leaf(l)){
                    uint32_t idx = leaf_idx(l);
                    T val = lf[idx].value;
                    int rem = lf[idx].count - p;
                    lf[idx].count = p;
                    uint32_t rnode = pool.make_leaf(val, rem);
                    rl = l; rr = rnode;
                    break;
                }
                uint32_t c0 = in[l].child[0], c1 = in[l].child[1];
                int c0_cnt = node_cnt(in, lf, c0);
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
            in = pool.inodes.data();
            lf = pool.leaves.data();
            for(int i = sp - 1; i >= 0; --i){
                uint32_t node = st[i].node;
                if(st[i].dir == 0){
                    in[node].child[0] = rr;
                    uint32_t c1 = in[node].child[1];
                    in[node].set_cnt(node_cnt(in, lf, rr) + node_cnt(in, lf, c1));
                    rr = node;
                } else {
                    in[node].child[1] = rl;
                    uint32_t c0 = in[node].child[0];
                    in[node].set_cnt(node_cnt(in, lf, c0) + node_cnt(in, lf, rl));
                    rl = node;
                }
            }
            return {rl, rr};
        }
        
        static DropRes split_drop_at_raw(Pool &pool, uint32_t l, int p){
            auto *in = pool.inodes.data();
            auto *lf = pool.leaves.data();
            struct Frame { uint32_t node; uint8_t dir; };
            Frame st[65];
            int sp = 0;
            while(!is_leaf(l)){
                uint32_t c0 = in[l].child[0];
                int c0_cnt = node_cnt(in, lf, c0);
                if(p < c0_cnt){
                    st[sp++] = {l, 0};
                    l = c0;
                } else {
                    st[sp++] = {l, 1};
                    p -= c0_cnt;
                    l = in[l].child[1];
                }
            }
            uint32_t rl = NIL, rr = NIL, dropped = NIL;
            uint32_t idx = leaf_idx(l);
            int c = lf[idx].count;
            if(c == 1){
                dropped = l;
            } else if(p == 0){
                lf[idx].count = c - 1;
                rr = l;
            } else if(p == c - 1){
                lf[idx].count = c - 1;
                rl = l;
            } else {
                T val = lf[idx].value;
                lf[idx].count = p;
                rr = pool.make_leaf(val, c - p - 1);
                rl = l;
                in = pool.inodes.data();
                lf = pool.leaves.data();
            }
            for(int i = sp - 1; i >= 0; --i){
                uint32_t node = st[i].node;
                if(st[i].dir == 0){
                    uint32_t c1 = in[node].child[1];
                    if(rr == NIL){
                        pool.push_inode(node);
                        rr = c1;
                    } else {
                        in[node].child[0] = rr;
                        in[node].set_cnt(node_cnt(in, lf, rr) + node_cnt(in, lf, c1));
                        rr = node;
                    }
                } else {
                    uint32_t c0 = in[node].child[0];
                    if(rl == NIL){
                        pool.push_inode(node);
                        rl = c0;
                    } else {
                        in[node].child[1] = rl;
                        in[node].set_cnt(node_cnt(in, lf, c0) + node_cnt(in, lf, rl));
                        rl = node;
                    }
                }
            }
            return {rl, dropped, rr};
        }
        
        binary_trie split_one(){
            if(root == NIL) return binary_trie();
            auto &pool = get_pool();
            bool old_rev = rev;
            uint32_t old = take_root();
            if(old_rev){
                return binary_trie(root_tag{}, split_one_dir<1>(pool, old), true);
            } else {
                return binary_trie(root_tag{}, split_one_dir<0>(pool, old));
            }
        }
        template<int DIR>
        static uint32_t split_one_dir(Pool &pool, uint32_t l){
            auto *in = pool.inodes.data();
            auto *lf = pool.leaves.data();
            uint32_t st[65];
            int sp = 0;
            uint32_t rem = NIL;
            while(true){
                if(is_leaf(l)){
                    uint32_t idx = leaf_idx(l);
                    if(lf[idx].count == 1){
                        pool.push_leaf(l);
                        rem = NIL;
                    } else {
                        lf[idx].count--;
                        rem = l;
                    }
                    break;
                }
                in[l].dec_cnt();
                st[sp++] = l;
                l = in[l].child[DIR];
            }
            uint32_t cur = rem;
            for(int i = sp - 1; i >= 0; --i){
                uint32_t p = st[i];
                if(cur == NIL){
                    cur = in[p].child[DIR ^ 1];
                    pool.push_inode(p);
                } else {
                    in[p].child[DIR] = cur;
                    cur = p;
                }
            }
            return cur;
        }
        
        void init(T p){
            rev = false;
            if(root != NIL){
                if(is_leaf(root)){
                    get_pool().leaves[leaf_idx(root)] = leaf_t(p, 1);
                    return;
                }
                clear();
            }
            root = get_pool().make_leaf(p, 1);
        }
        
        static int get_size(uint32_t ptr){
            if(ptr == NIL) return 0;
            if(is_leaf(ptr)) return 1;
            const auto &in = get_pool().inodes;
            return 1 + get_size(in[ptr].child[0]) + get_size(in[ptr].child[1]);
        }
    };
    
    int n;
    vector<binary_trie> trie;
    fastset fset;
    vector<uint32_t> merge_roots;
    vector<typename binary_trie::LeafItem> leaf_buf, leaf_tmp;
    
    sortable_array(int n) : n(n), trie(n), fset(n+1, false) {
        binary_trie::reserve(static_cast<size_t>(n));
        fset.insert(0);
        if(n > 0){
            fset.insert(n);
            trie[0].set_root(binary_trie::get_pool().make_leaf(T(0), n), false);
        }
    }
    sortable_array(const vector<T> &v) : n(v.size()), trie(n), fset(n+1, true) {
        binary_trie::reserve(static_cast<size_t>(n));
        auto &pool = binary_trie::get_pool();
        for(int i = 0; i < n; i++) trie[i].set_root(pool.make_leaf(v[i], 1), false);
    }
    sortable_array(sortable_array<T> &&o) = default;
    sortable_array& operator=(sortable_array<T> &&o) = default;
    
    void set(int p, T x){
        int l = fset.less_bound(p);
        int sz = trie[l].size();
        int r = l + sz;
        if(sz == 1){
            trie[p].init(x);
            return;
        }
        auto &pool = binary_trie::get_pool();
        if(trie[l].is_single_leaf() && pool.leaves[binary_trie::leaf_idx(trie[l].peek_root())].value == x){
            return;
        }
        bool old_rev = trie[l].rev;
        uint32_t old = trie[l].take_root();
        int pos = old_rev ? (sz - 1 - (p - l)) : (p - l);
        auto dr = binary_trie::split_drop_at_raw(pool, old, pos);
        uint32_t left_rt = old_rev ? dr.right : dr.left;
        uint32_t right_rt = old_rev ? dr.left : dr.right;
        uint32_t mid_rt = dr.dropped;
        if(mid_rt != binary_trie::NIL){
            pool.leaves[binary_trie::leaf_idx(mid_rt)] = typename binary_trie::leaf_t(x, 1);
        } else {
            mid_rt = pool.make_leaf(x, 1);
        }
        if(l < p){
            trie[l].set_root(left_rt, old_rev);
            fset.insert(p);
        }
        trie[p].set_root(mid_rt, false);
        if(p + 1 < r){
            trie[p + 1].set_root(right_rt, old_rev);
            fset.insert(p + 1);
        }
    }
    
    T get(int p) const {
        int l = fset.less_bound(p);
        int idx = p - l;
        return trie[l].rev ? trie[l][trie[l].size()-idx-1] : trie[l][idx];
    }
    
    void _sort(int l, int r, bool is_rev) {
        if(r - l <= 1) return;
        int lb = fset.less_bound(l);
        int lbr = lb + trie[lb].size();
        if(r <= lbr){
            if(trie[lb].rev == is_rev || trie[lb].is_single_leaf()) return;
            if(lb == l && r == lbr){
                trie[l].rev = is_rev;
                return;
            }
            if(lb < l){
                auto res = trie[lb].split(l-lb);
                trie[lb] = move(res.first); trie[l] = move(res.second);
                fset.insert(l);
            }
            if(r < lbr){
                auto res = trie[l].split(r-l);
                trie[l] = move(res.first); trie[r] = move(res.second);
                fset.insert(r);
            }
            trie[l].rev = is_rev;
            return;
        }
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
        auto &pool = binary_trie::get_pool();
        merge_roots.clear();
        merge_roots.push_back(trie[l].take_root());
        fset.for_each_and_erase(l + 1, r, [&](int b){
            merge_roots.push_back(trie[b].take_root());
        });
        uint32_t cur;
        int k = static_cast<int>(merge_roots.size());
        if(k == 1){
            cur = merge_roots[0];
        } else if(k == 2){
            cur = binary_trie::meld(pool, merge_roots[0], merge_roots[1]);
        } else {
            int leaf_cnt = 0;
            for(uint32_t rt : merge_roots) if(binary_trie::is_leaf(rt)) ++leaf_cnt;
            if(leaf_cnt >= 16){
                auto *lf = pool.leaves.data();
                leaf_buf.clear();
                int rem = 0;
                for(uint32_t rt : merge_roots){
                    if(binary_trie::is_leaf(rt)) leaf_buf.push_back({lf[binary_trie::leaf_idx(rt)].value, rt});
                    else merge_roots[rem++] = rt;
                }
                merge_roots.resize(rem);
                merge_roots.push_back(binary_trie::build_from_leaves(pool, leaf_buf, leaf_tmp));
                k = static_cast<int>(merge_roots.size());
            }
            cur = merge_roots[0];
            for(int i = 1; i < k; ++i) cur = binary_trie::meld(pool, cur, merge_roots[i]);
        }
        trie[l].set_root(cur, is_rev);
    }
    
    void sort(int l, int r) { _sort(l, r, false); }
    
    void sort_desc(int l, int r) { _sort(l, r, true); }
};
