auto inversion_number = [](const auto& v){
    int n = static_cast<int>(ssize(v));
    if(n <= 1) return 0LL;
    using T = decay_t<decltype(*begin(v))>;
    if constexpr (is_integral_v<T> && sizeof(T) <= 8) {
        T mn = *begin(v), mx = mn;
        for(const auto& x : v){
            if(x < mn) mn = x;
            if(x > mx) mx = x;
        }
        using U = make_unsigned_t<conditional_t<is_same_v<T, bool>, unsigned char, T>>;
        unsigned long long diff = static_cast<unsigned long long>(static_cast<U>(mx) - static_cast<U>(mn));
        if(diff <= static_cast<unsigned long long>(n) * 2ULL){
            int m = static_cast<int>(diff) + 1;
            vector<int> bit(m + 1, 0);
            long long res = 0;
            int cnt = 0;
            for(const auto& x : v){
                int id = static_cast<int>(static_cast<U>(x) - static_cast<U>(mn)) + 1;
                int s = 0;
                for(int p = id; p > 0; p -= p & -p) s += bit[p];
                res += cnt - s;
                for(int p = id; p <= m; p += p & -p) bit[p]++;
                ++cnt;
            }
            return res;
        }
    }
    vector<T> vals(begin(v), end(v));
    sort(vals.begin(), vals.end());
    vals.erase(unique(vals.begin(), vals.end()), vals.end());
    int m = static_cast<int>(vals.size());
    vector<int> bit(m + 1, 0);
    auto add = [&](int p){
        for(; p <= m; p += p & -p) bit[p]++;
    };
    auto sum = [&](int p){
        int s = 0;
        for(; p > 0; p -= p & -p) s += bit[p];
        return s;
    };
    long long res = 0;
    int cnt = 0;
    for(const auto& x : v){
        int id = static_cast<int>(lower_bound(vals.begin(), vals.end(), x) - vals.begin()) + 1;
        res += cnt - sum(id);
        add(id);
        ++cnt;
    }
    return res;
};