template<typename T>
struct graph {
    vector<T> G;
    vector<int> idx;
    struct ref_t {
        vector<T>::iterator begin_, end_;
        auto begin() const noexcept {return begin_;}
        auto end() const noexcept {return end_;}
        auto size() const noexcept {return end_ - begin_;}
        T operator[](int p) const {return begin_[p];}
    };
    
    graph() = default;
    graph(int n, const auto &E){
        build(n, E);
    }
    
    void build(int n, const auto &E){
        G.resize(E.size());
        idx.resize(n+1);
        for(auto &[u, v] : E){
            idx[u+1]++;
        }
        for(int i = 1; i <= n; i++) idx[i] += idx[i-1];
        auto C = idx;
        for(auto &[u, v] : E){
            G[C[u]++] = v;
        }
    }
    
    ref_t operator[](int p) {return {G.begin()+idx[p], G.begin()+idx[p+1]};}
};
