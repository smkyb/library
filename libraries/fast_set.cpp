// 注意: 値域 [0, n), GCC/Clang 専用; insert のみ [0, n) を assert (erase/count は範囲外を無視)
//       max_leq(x) は x 以下 (x を含む) の最大値を返し、該当なし時は lower_bound / max_leq ともに -1
struct fast_set{
    using ull = unsigned long long;
    using uint = unsigned;
    
    int n_ = 0;
    int siz = 0;
    vector<ull> node;
    
    fast_set(int n = 0, bool fill = false) : n_(n) {
        assert(n >= 0);
        if(n == 0) return;
        ull cap = 1;
        while((cap << 6) < static_cast<ull>(n)) cap <<= 6;
        siz = static_cast<int>((cap + 1) / 63);
        node.resize(static_cast<size_t>(siz) + (static_cast<size_t>(n) + 63) / 64);
        if(fill){
            uint cur = static_cast<uint>(siz);
            uint len = static_cast<uint>((static_cast<ull>(n) + 63) >> 6);
            uint rem = static_cast<uint>(n) & 63u;
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
        assert(0 <= x && x < n_);
        uint idx = static_cast<uint>(siz) + (static_cast<uint>(x) >> 6);
        uint b = static_cast<uint>(x) & 63u;
        while(true){
            if((node[idx] >> b) & 1ull) return;
            node[idx] |= (1ull << b);
            if(idx == 0) return;
            idx--;
            b = idx & 63u;
            idx >>= 6;
        }
    }
    
    void erase(int x){
        if(x < 0 || x >= n_) return;
        uint idx = static_cast<uint>(siz) + (static_cast<uint>(x) >> 6);
        uint b = static_cast<uint>(x) & 63u;
        if(!((node[idx] >> b) & 1ull)) return;
        while(true){
            node[idx] &= ~(1ull << b);
            if(idx == 0 || node[idx]) return;
            idx--;
            b = idx & 63u;
            idx >>= 6;
        }
    }
    
    bool count(int x) const {
        if(x < 0 || x >= n_) return false;
        return (node[static_cast<size_t>(siz) + (static_cast<size_t>(x) >> 6)] >> (static_cast<uint>(x) & 63u)) & 1ull;
    }
    
    int lower_bound(int x) const {
        if(x >= n_ || n_ == 0) return -1;
        if(x < 0) x = 0;
        uint idx = static_cast<uint>(siz) + (static_cast<uint>(x) >> 6);
        uint b = static_cast<uint>(x) & 63u;
        ull mask = node[idx] & (~0ull << b);
        if(mask) return static_cast<int>((idx - static_cast<uint>(siz)) * 64 + __builtin_ctzll(mask));
        while(idx > 0){
            idx--;
            b = idx & 63u;
            idx >>= 6;
            if(b < 63u && (mask = node[idx] & (~1ull << b))){
                b = static_cast<uint>(__builtin_ctzll(mask));
                idx = (idx << 6) + b + 1;
                while(idx < static_cast<uint>(siz)) idx = (idx << 6) + static_cast<uint>(__builtin_ctzll(node[idx])) + 1;
                return static_cast<int>((idx - static_cast<uint>(siz)) * 64 + __builtin_ctzll(node[idx]));
            }
        }
        return -1;
    }
    
    int max_leq(int x) const {
        if(x < 0 || n_ == 0) return -1;
        if(x >= n_) x = n_ - 1;
        uint idx = static_cast<uint>(siz) + (static_cast<uint>(x) >> 6);
        uint b = static_cast<uint>(x) & 63u;
        ull mask = node[idx] & (~0ull >> (63u - b));
        if(mask) return static_cast<int>((idx - static_cast<uint>(siz)) * 64 + 63 - __builtin_clzll(mask));
        while(idx > 0){
            idx--;
            b = idx & 63u;
            idx >>= 6;
            if(b > 0 && (mask = node[idx] & ((1ull << b) - 1ull))){
                b = static_cast<uint>(63 - __builtin_clzll(mask));
                idx = (idx << 6) + b + 1;
                while(idx < static_cast<uint>(siz)) idx = (idx << 6) + 64 - static_cast<uint>(__builtin_clzll(node[idx]));
                return static_cast<int>((idx - static_cast<uint>(siz)) * 64 + 63 - __builtin_clzll(node[idx]));
            }
        }
        return -1;
    }
};