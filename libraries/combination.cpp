// 注意: mod は素数かつ max_n < mod 必須; 有効範囲 (0 <= k <= n) で n <= max_n を assert (範囲外は 0)
template<int mod, int max_n>
struct combination {
    static_assert(max_n >= 0 && max_n < mod);
    inline static int fact[max_n+1], finv[max_n+1], inv[max_n+1];
    
    combination() = default;
    
    static void init(){
        fact[0] = 1; finv[0] = 1;
        if constexpr (max_n >= 1) {
            fact[1] = 1; finv[1] = 1; inv[1] = 1;
        }
        for(int i = 2; i <= max_n; i++) fact[i] = (long long)fact[i-1] * i % mod;
        for(int i = 2; i <= max_n; i++) inv[i] = mod - (long long)inv[mod%i] * (mod/i) % mod;
        for(int i = 2; i <= max_n; i++) finv[i] = (long long)finv[i-1] * inv[i] % mod;
    }
    
    inline long long operator()(int n, int k) const {
        if(n < k || n < 0 || k < 0) return 0;
        assert(n <= (int)max_n);
        if(fact[0] == 0) [[unlikely]] init();
        return (long long)fact[n] * finv[k] % mod * finv[n-k] % mod;
    }
};