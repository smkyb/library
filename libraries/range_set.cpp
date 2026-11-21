// 注意: 整数型 T 専用 (lower_bound が -1 を返すため符号付き型・非負値を前提)
//       区間版 insert/erase/contains(l, r) は半開 [l, r) のため T::max() を含められない (単要素版を使うこと)
//       mex(x)/get_range(x) は T::max() 境界で overflow_error を投げ得る; get_range(x) は x の存在が必須
template<typename T = long long>
struct range_set{
    map<T, T> S;
    // 最大値だけは [x, x+1) の右端が T に収まらないため別に持つ。
    bool has_max = false;
    
    void insert(T x){
        if(x == numeric_limits<T>::max()) has_max = true;
        else insert(x, x + 1);
    }
    
    void insert(T l, T r){
        if(l >= r) return;
        auto it = S.lower_bound(l);
        T cur_r = r;
        if(it != S.begin()){
            auto pit = prev(it);
            if(pit->second >= l){
                if(pit->second >= r) return;
                while(it != S.end() && it->first <= cur_r){
                    if(it->second > cur_r) cur_r = it->second;
                    it = S.erase(it);
                }
                pit->second = cur_r;
                return;
            }
        }
        while(it != S.end() && it->first <= cur_r){
            if(it->second > cur_r) cur_r = it->second;
            it = S.erase(it);
        }
        S.emplace_hint(it, l, cur_r);
    }
    
    void erase(T x){
        if(x == numeric_limits<T>::max()) has_max = false;
        else erase(x, x + 1);
    }
    
    void erase(T l, T r){
        if(l >= r) return;
        auto it = S.lower_bound(l);
        if(it != S.begin()){
            auto pit = prev(it);
            if(pit->second > l){
                T old_r = pit->second;
                pit->second = l;
                if(old_r > r){
                    S.emplace_hint(it, r, old_r);
                    return;
                }
            }
        }
        while(it != S.end() && it->first < r){
            T old_r = it->second;
            it = S.erase(it);
            if(old_r > r){
                S.emplace_hint(it, r, old_r);
                return;
            }
        }
    }
    
    T lower_bound(T x) const {
        auto it = S.lower_bound(x);
        if(it != S.begin() && prev(it)->second > x) return x;
        if(it != S.end()) return it->first;
        if(has_max) return numeric_limits<T>::max();
        return -1;
    }
    
    // 型の範囲に未登録の値が残っていなければ例外。
    T mex(T x) const {
        T res = x;
        auto it = S.upper_bound(x);
        if(it != S.begin()){
            auto pit = prev(it);
            if(pit->second > x) res = pit->second;
        }
        if(has_max && res == numeric_limits<T>::max()) throw overflow_error("range_set::mex");
        return res;
    }
    
    bool contains(T x) const {
        if(x == numeric_limits<T>::max()) return has_max;
        auto it = S.upper_bound(x);
        return it != S.begin() && prev(it)->second > x;
    }
    
    bool contains(T l, T r) const {
        if(l >= r) return true;
        auto it = S.upper_bound(l);
        return it != S.begin() && prev(it)->second >= r;
    }
    
    // 区間に型の最大値を含む場合は半開区間の右端を返せないため例外。
    pair<T, T> get_range(T x) const {
        constexpr T max_value = numeric_limits<T>::max();
        if(x == max_value && has_max) throw overflow_error("range_set::get_range");
        auto it = S.upper_bound(x);
        assert(it != S.begin() && prev(it)->second > x);
        if(has_max && prev(it)->second == max_value) throw overflow_error("range_set::get_range");
        return *prev(it);
    }
};