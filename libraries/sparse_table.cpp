template<typename T, auto op, auto e>
struct sparse_table{
    int n = 0;
    vector<T> table;
    vector<int> start;
    
    sparse_table() = default;
    sparse_table(const vector<T>& v) : n(static_cast<int>(v.size())) {
        if(v.empty()) return;
        int l2n = 31 - __builtin_clz(static_cast<unsigned>(n));
        start.resize(l2n + 1);
        start[0] = 0;
        size_t total = static_cast<size_t>(n);
        for(int bi = 1; bi <= l2n; ++bi){
            start[bi] = static_cast<int>(total);
            total += static_cast<size_t>(n - (1 << bi) + 1);
        }
        table.reserve(total);
        table.assign(v.begin(), v.end());
        table.resize(total);
        for(int bi = 1; bi <= l2n; ++bi){
            int half = 1 << (bi - 1);
            int len = n - (1 << bi) + 1;
            const T* prev = table.data() + start[bi - 1];
            T* cur = table.data() + start[bi];
            for(int i = 0; i < len; ++i){
                cur[i] = op(prev[i], prev[i + half]);
            }
        }
    }
    
    T prod(int l, int r) const {
        assert(0 <= l && l <= r && r <= n);
        if(l == r) return e();
        int bi = 31 - __builtin_clz(static_cast<unsigned>(r - l));
        return op(table[start[bi] + l], table[start[bi] + r - (1 << bi)]);
    }
};