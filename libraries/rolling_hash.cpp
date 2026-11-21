// 注意: C++17 以降・61bit 単一ハッシュ (衝突対策が必要なら複基数化); get(l, r) は半開 [l, r)
//       string / vector<char> (+1) とその他の vector<T> (+10^9+7) で符号化が異なるため混在比較不可
struct rolling_hash{
    using ull = unsigned long long;
    static constexpr ull mod = (1ULL << 61) - 1;
    
    static ull gen_base(){
        random_device rd;
        auto t = static_cast<ull>(chrono::steady_clock::now().time_since_epoch().count());
        seed_seq seq{rd(), rd(), rd(), rd(), static_cast<unsigned>(t), static_cast<unsigned>(t >> 32)};
        mt19937_64 rng(seq);
        return uniform_int_distribution<ull>(1000, mod - 2)(rng);
    }
    inline static const ull base = gen_base();
    
    static inline ull calc_mod(ull x) noexcept {
        ull xu = x >> 61, xd = x & mod;
        ull res = xu + xd;
        if(res >= mod) res -= mod;
        return res;
    }
    static inline ull mul(ull a, ull b) noexcept {
#ifdef __SIZEOF_INT128__
        __uint128_t t = static_cast<__uint128_t>(a) * b;
        ull xu = static_cast<ull>(t >> 61), xd = static_cast<ull>(t & mod);
        ull res = xu + xd;
        if(res >= mod) res -= mod;
        return res;
#else
        ull au = a >> 31, ad = a & ((1ULL << 31) - 1);
        ull bu = b >> 31, bd = b & ((1ULL << 31) - 1);
        ull mid = ad * bu + au * bd;
        ull midu = mid >> 30, midd = mid & ((1ULL << 30) - 1);
        return calc_mod(au * bu * 2 + midu + (midd << 31) + ad * bd);
#endif
    }
    
    inline static vector<ull> pow_{1};
    static void ensure_pow(int len){
        int cur = static_cast<int>(pow_.size());
        if(cur <= len){
            pow_.resize(len + 1);
            for(int i = cur; i <= len; ++i) pow_[i] = mul(pow_[i - 1], base);
        }
    }
    
    int n = 0;
    vector<ull> hashed;
    
    rolling_hash() = default;
    rolling_hash(const string &s) : n(static_cast<int>(s.size())), hashed(n + 1) {
        ensure_pow(n);
        for(int i = 0; i < n; i++){
            ull c = static_cast<unsigned char>(s[i]) + 1ULL;
            hashed[i + 1] = calc_mod(mul(hashed[i], base) + c);
        }
    }
    template<typename T>
    rolling_hash(const vector<T> &v) : n(static_cast<int>(v.size())), hashed(n + 1) {
        ensure_pow(n);
        for(int i = 0; i < n; i++){
            ull c;
            if constexpr (is_same_v<T, char>) {
                c = static_cast<unsigned char>(v[i]) + 1ULL;
            } else {
                ull u = (is_signed_v<T> && v[i] < 0)
                    ? (mod - (0ULL - static_cast<ull>(v[i])) % mod)
                    : (static_cast<ull>(v[i]) % mod);
                if(u >= mod) u -= mod;
                c = u + 1000000007ULL;
                if(c >= mod) c -= mod;
            }
            hashed[i + 1] = calc_mod(mul(hashed[i], base) + c);
        }
    }
    
    ull get(int l, int r) const {
        assert(0 <= l && r <= n);
        if(l >= r) return 0ULL;
        ull sub = mul(hashed[l], pow_[r - l]);
        ull res = hashed[r] + mod - sub;
        if(res >= mod) res -= mod;
        return res;
    }
};