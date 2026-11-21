// 注意: 0-indexed の有向 CSR (無向グラフは両方向を edges に含めること); 頂点数 n と辺数 |edges| は int 範囲内・C++20
//       g[u] が返すイテレータビューは build や csr_graph 破棄で無効化される
template<typename T>
struct csr_graph {
    vector<T> G;
    vector<int> idx;
    struct ref_t {
        typename vector<T>::iterator begin_, end_;
        auto begin() const noexcept {return begin_;}
        auto end() const noexcept {return end_;}
        auto size() const noexcept {return end_ - begin_;}
        T& operator[](int p) const {return begin_[p];}
    };
    struct const_ref_t {
        typename vector<T>::const_iterator begin_, end_;
        auto begin() const noexcept {return begin_;}
        auto end() const noexcept {return end_;}
        auto size() const noexcept {return end_ - begin_;}
        const T& operator[](int p) const {return begin_[p];}
    };
    
    csr_graph() = default;
    csr_graph(int n, const auto &edges){
        build(n, edges);
    }
    
    void build(int n, const auto &edges){
        assert(n >= 0);
        G.resize(edges.size());
        idx.assign(n+1, 0);
        for(const auto &[u, v] : edges){
            assert(0 <= u && u < n);
            idx[u+1]++;
        }
        for(int i = 1; i <= n; i++) idx[i] += idx[i-1];
        auto C = idx;
        for(const auto &[u, v] : edges){
            G[C[u]++] = v;
        }
    }
    
    int size() const noexcept {return idx.empty() ? 0 : static_cast<int>(idx.size()) - 1;}
    
    ref_t operator[](int u) {
        assert(0 <= u && u + 1 < static_cast<int>(idx.size()));
        return {G.begin()+idx[u], G.begin()+idx[u+1]};
    }
    const_ref_t operator[](int u) const {
        assert(0 <= u && u + 1 < static_cast<int>(idx.size()));
        return {G.cbegin()+idx[u], G.cbegin()+idx[u+1]};
    }
};