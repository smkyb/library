// 注意: C++17 以降 (GCC/Clang); 入力は unsigned long long (< 2^64) まで対応 (それ以上は切り捨て)
//       n < 2 の場合は空配列 {} を返す
auto factorize = [](auto n){
    using T = decltype(n);
    using U = unsigned long long;
    vector<pair<T, int>> res;
    if(n < 2) return res;
    U m = static_cast<U>(n);
    if((m & 1ULL) == 0){
        int c = __builtin_ctzll(m);
        res.emplace_back(static_cast<T>(2), c);
        m >>= c;
    }
    static constexpr U sp[] = {3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};
    for(U p : sp){
        if(m % p == 0){
            int c = 0;
            do { ++c; m /= p; } while(m % p == 0);
            res.emplace_back(static_cast<T>(p), c);
        }
        if(m == 1) return res;
    }
    if(m < 41ULL * 41ULL){
        res.emplace_back(static_cast<T>(m), 1);
        return res;
    }
    auto mul_mod = [](U a, U b, U mod) -> U {
#ifdef __SIZEOF_INT128__
        return static_cast<U>(static_cast<__uint128_t>(a) * b % mod);
#else
        U r = 0;
        a %= mod;
        while(b){
            if(b & 1ULL) r = (r >= mod - a) ? (r - (mod - a)) : (r + a);
            a = (a >= mod - a) ? (a - (mod - a)) : (a + a);
            b >>= 1;
        }
        return r;
#endif
    };
    auto pow_mod = [&](U a, U e, U mod) -> U {
        U r = 1;
        a %= mod;
        while(e){
            if(e & 1ULL) r = mul_mod(r, a, mod);
            a = mul_mod(a, a, mod);
            e >>= 1;
        }
        return r;
    };
    auto is_prime = [&](U x) -> bool {
        if(x < 2) return false;
        for(U p : {2ULL, 3ULL, 5ULL, 7ULL, 11ULL, 13ULL, 17ULL, 19ULL, 23ULL, 29ULL, 31ULL, 37ULL}){
            if(x == p) return true;
            if(x % p == 0) return false;
        }
        if(x < 41ULL * 41ULL) return true;
        U d = x - 1;
        int s = __builtin_ctzll(d);
        d >>= s;
        auto check_base = [&](U a) -> bool {
            U b = a % x;
            if(b == 0) return true;
            U v = pow_mod(b, d, x);
            if(v == 1 || v == x - 1) return true;
            for(int r = 1; r < s; ++r){
                v = mul_mod(v, v, x);
                if(v == x - 1) return true;
            }
            return false;
        };
        if(x < 4759123141ULL){
            for(U a : {2ULL, 7ULL, 61ULL}) if(!check_base(a)) return false;
        } else {
            for(U a : {2ULL, 325ULL, 9375ULL, 28178ULL, 450775ULL, 9780504ULL, 1795265022ULL}){
                if(!check_base(a)) return false;
            }
        }
        return true;
    };
    auto find_factor = [&](U x) -> U {
        if((x & 1ULL) == 0) return 2;
        for(U c = 1; ; ++c){
            U cc = c % x;
            if(cc == 0) cc = 1;
            auto f = [&](U v) -> U {
                U t = mul_mod(v, v, x);
                return (t >= x - cc) ? (t - (x - cc)) : (t + cc);
            };
            U y = (c % (x - 1)) + 1;
            U g = 1, q = 1, r = 1, ys = 0, mx = 0;
            while(g == 1){
                mx = y;
                for(U i = 0; i < r; ++i) y = f(y);
                U k = 0;
                while(k < r && g == 1){
                    ys = y;
                    U lim = min(128ULL, r - k);
                    for(U i = 0; i < lim; ++i){
                        y = f(y);
                        U diff = (mx >= y) ? (mx - y) : (y - mx);
                        q = mul_mod(q, diff, x);
                    }
                    g = std::gcd(q, x);
                    k += lim;
                }
                r <<= 1;
            }
            if(g == x){
                do {
                    ys = f(ys);
                    U diff = (mx >= ys) ? (mx - ys) : (ys - mx);
                    g = std::gcd(diff, x);
                } while(g == 1);
            }
            if(g < x) return g;
        }
    };
    vector<U> facs, st{m};
    while(!st.empty()){
        U cur = st.back();
        st.pop_back();
        if(cur == 1) continue;
        if(is_prime(cur)){
            facs.push_back(cur);
        } else {
            U d = find_factor(cur);
            st.push_back(d);
            st.push_back(cur / d);
        }
    }
    sort(facs.begin(), facs.end());
    for(U p : facs){
        if(res.empty() || res.back().first != static_cast<T>(p)) res.emplace_back(static_cast<T>(p), 1);
        else ++res.back().second;
    }
    return res;
};