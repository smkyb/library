// 注意: 0-indexed・C++20 (bit_floor) 必須; sum / prod は半開区間 [l, r) / [0, r)
//       max_right(x) は全要素非負が前提; 部分和が signed T でオーバーフローしないこと
template<typename T>
struct fenwick_tree{
private:
    // 符号付き整数は途中の加減算を符号なしで行い、最後に T へ戻す。
    static auto zero_accumulator(){
        if constexpr (is_integral_v<T> && is_signed_v<T>) {
            return common_type_t<make_unsigned_t<T>, unsigned>{0};
        } else {
            T res = 0;
            return res;
        }
    }

public:
    int n;
    vector<T> node;
    
    fenwick_tree() = default;
    fenwick_tree(int n):
        n(n), node((assert(n >= 0), n+1)){}
    fenwick_tree(const vector<T>& v):
        n(static_cast<int>(v.size())), node(n+1){
        for(int i = 0; i < n; i++) node[i+1] = v[i];
        for(int i = 1; i < n; i++) if(i+(i&-i) <= n) node[i+(i&-i)] += node[i];
    }
    
    void add(int p, const T x){
        assert(0 <= p && p < n);
        p++;
        for(; p <= n; p += p&(-p)){
            node[p] += x;
        }
    }
    
    T sum(int l, int r) const {
        assert(0 <= l && l <= r && r <= n);
        auto res = zero_accumulator();
        while(r > l){
            res += node[r];
            r -= r&(-r);
        }
        while(l > r){
            res -= node[l];
            l -= l&(-l);
        }
        return res;
    }
    
    T sum(int r) const {
        assert(0 <= r && r <= n);
        auto res = zero_accumulator();
        for(; r > 0; r -= r&(-r)){
            res += node[r];
        }
        return res;
    }
    
    T prod(int l, int r) const {
        return sum(l, r);
    }
    
    T get(int p) const {
        assert(0 <= p && p < n);
        return sum(p, p + 1);
    }
    
    void set(int p, const T x){
        if constexpr (is_integral_v<T> && is_signed_v<T>) {
            using U = decltype(zero_accumulator());
            U delta = U(x) - U(get(p));
            for(++p; p <= n; p += p&(-p)) node[p] = static_cast<T>(U(node[p]) + delta);
        } else {
            add(p, x - get(p));
        }
    }
    
    int max_right(T x) const {
        if(x < 0) return 0;
        int res = 0, w = bit_floor(static_cast<unsigned>(n));
        for(int len = w; len > 0; len >>= 1){
            if(len + res <= n && node[len + res] <= x){
                x -= node[len + res];
                res += len;
            }
        }
        return res;
    }
};