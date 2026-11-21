// 注意: 関係式は potential[x] - potential[y] == w; 保存する重みと取得する差が型 S で表現可能であること
//       diff(x, y) は same(x, y) == true が前提 (範囲チェックなし)
template<class S> struct weighted_union_find {
    vector<int> d;
    vector<S> weight;
    
    weighted_union_find(int n = 0):
        d(n, -1), weight(n) {}
    
    int leader(int x){
        int st[64];
        int sp = 0;
        while(d[x] >= 0){
            st[sp++] = x;
            x = d[x];
        }
        while(sp > 0){
            int u = st[--sp];
            weight[u] += weight[d[u]];
            d[u] = x;
        }
        return x;
    }
    
    S potential(int x){
        leader(x);
        return weight[x];
    }
    
    bool same(int x, int y){
        return leader(x) == leader(y);
    }
    
    // x - y == w。保存する重みと取得する差は S で表現できること。
    bool merge(int x, int y, S w){
        int rx = leader(x), ry = leader(y);
        if constexpr (is_integral_v<S> && is_signed_v<S>) {
            if(rx == ry){
                S diff;
                return !__builtin_sub_overflow(weight[x], weight[y], &diff) && diff == w;
            }
            // 符号を確定した子の重みだけを S に戻す。
            using U = common_type_t<make_unsigned_t<S>, unsigned>;
            U delta = U(w) + U(weight[y]) - U(weight[x]);
            if(d[rx] > d[ry]){
                swap(rx, ry);
            } else {
                delta = U(0) - delta;
            }
            weight[ry] = static_cast<S>(delta);
        } else {
            if constexpr (is_floating_point_v<S>) {
                S diff = weight[y] - weight[x];
                if(isinf(diff) && isfinite(weight[y]) && isfinite(weight[x])) [[unlikely]] {
                    // 差分があふれる場合だけ、打ち消し合う項を先に計算する。
                    w = signbit(w) == signbit(weight[x])
                        ? (w - weight[x]) + weight[y] : (w + weight[y]) - weight[x];
                } else {
                    w += diff;
                }
            } else {
                w += weight[y] - weight[x];
            }
            if(rx == ry) return w == S(0);
            if(d[rx] > d[ry]) swap(rx, ry), w = -w;
            weight[ry] = -w;
        }
        d[rx] += d[ry];
        d[ry] = rx;
        return true;
    }
    
    S diff(int x, int y){
        return potential(x) - potential(y);
    }
};