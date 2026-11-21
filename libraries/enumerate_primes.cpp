auto enumerate_primes = [](int n){
    if(n < 2) return vector<int>(0);
    vector<int> prime;
    if(n > 1000) prime.reserve(static_cast<size_t>(n / (log(static_cast<double>(n)) - 1.1)));
    prime.push_back(2);
    if(n >= 3) prime.push_back(3);
    if(n >= 5) prime.push_back(5);
    if(n < 7) return prime;
    
    static constexpr int R[8] = {1, 7, 11, 13, 17, 19, 23, 29};
    static constexpr int R_INV[30] = {
        -1, 0, -1, -1, -1, -1, -1, 1, -1, -1,
        -1, 2, -1, 3, -1, -1, -1, 4, -1, 5,
        -1, -1, -1, 6, -1, -1, -1, -1, -1, 7
    };
    
    long long lim_ll = static_cast<long long>(sqrt(static_cast<double>(n)));
    while((lim_ll + 1) * (lim_ll + 1) <= n) ++lim_ll;
    while(lim_ll * lim_ll > n) --lim_ll;
    int lim = static_cast<int>(lim_ll);
    
    struct SP {
        int p;
        int pos[8];
        uint8_t mask[8];
    };
    vector<SP> sps;
    vector<uint8_t> sm(lim / 2 + 1, 1);
    for(int i = 3; i <= lim; i += 2){
        if(sm[i >> 1]){
            if(i >= 7){
                SP sp;
                sp.p = i;
                int base = (i / 30) * 30;
                for(int j = 0; j < 8; ++j){
                    int m0 = base + R[j];
                    if(m0 < i) m0 += 30;
                    long long sv = static_cast<long long>(i) * m0;
                    sp.pos[j] = static_cast<int>(sv / 30);
                    sp.mask[j] = static_cast<uint8_t>(~(1u << R_INV[sv % 30]));
                }
                sps.push_back(sp);
            }
            if(static_cast<long long>(i) * i <= lim){
                for(int j = i * i; j <= lim; j += 2 * i) sm[j >> 1] = 0;
            }
        }
    }
    
    constexpr int S = 32768;
    vector<uint8_t> block(S);
    int total_bytes = n / 30 + 1;
    for(int low_byte = 0; low_byte < total_bytes; low_byte += S){
        int len = min(S, total_bytes - low_byte);
        memset(block.data(), 0xFF, static_cast<size_t>(len));
        if(low_byte == 0) block[0] &= static_cast<uint8_t>(~1u);
        long long high_val = 30LL * (low_byte + len) - 1;
        for(auto &sp : sps){
            int p = sp.p;
            if(static_cast<long long>(p) * p > high_val) break;
            for(int j = 0; j < 8; ++j){
                int idx = sp.pos[j] - low_byte;
                uint8_t m = sp.mask[j];
                for(; idx < len; idx += p) block[idx] &= m;
                sp.pos[j] = idx + low_byte;
            }
        }
        long long base_val = 30LL * low_byte;
        for(int b = 0; b < len; ++b, base_val += 30){
            unsigned mask = block[b];
            while(mask){
                int bit = __builtin_ctz(mask);
                long long val = base_val + R[bit];
                if(val > n) break;
                prime.push_back(static_cast<int>(val));
                mask &= mask - 1;
            }
        }
    }
    return prime;
};