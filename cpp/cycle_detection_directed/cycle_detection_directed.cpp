vector<int> cycle_detection_directed(int n, const auto &e) {
    struct graph { vector<array<int, 2>> G; vector<int> idx; struct ref_t { vector<array<int, 2>>::iterator begin_, end_; auto begin() const noexcept {return begin_;} auto end() const noexcept {return end_;} auto size() const noexcept {return end_ - begin_;} array<int, 2> operator[](int p) const {return begin_[p];} }; graph() = default; void build(int n, const vector<pair<int, array<int, 2>>> &E){ G.resize(E.size()); idx.resize(n+1); for(auto &[u, v] : E){ idx[u+1]++; } for(int i = 1; i <= n; i++) idx[i] += idx[i-1]; auto C = idx; for(auto &[u, v] : E){ G[C[u]++] = v; } } ref_t operator[](int p) {return {G.begin()+idx[p], G.begin()+idx[p+1]};} };
    int m = e.size();
    vector<pair<int, array<int, 2>>> E;
    E.reserve(m);
    for(auto &[u, v] : e){
        const int siz = E.size();
        E.push_back({u, {v, siz}});
    }
    graph G;
    G.build(n, E);
    
    vector<int> st; st.reserve(n);
    vector<int> ans; ans.reserve(n);
    vector<int> state(n), idx(n), par(n, -1), done(n);
    int pos, ret = -1;
    for(int start = 0; start < n; start++){
        if(state[start] != 0) continue;
        pos = start;
        while(pos != -1){
            auto g = G[pos];
            if(done[pos]){
                done[pos] = false;
                if(ret >= 0){
                    ans.push_back(st.back());
                    st.pop_back();
                    if(ret == pos) ret = -2;
                    pos = par[pos];
                    continue;
                } else if(ret == -2){
                    st.pop_back();
                    pos = par[pos];
                    continue;
                } else st.pop_back();
            }
            if(idx[pos] == (int)g.size()){
                state[pos] = 2;
                pos = par[pos];
                ret = -1;
            } else {
                state[pos] = 1;
                auto [nex, i] = g[idx[pos]];
                idx[pos]++;
                if(state[nex] == 2) continue;
                if(state[nex] == 1) {
                    ans.push_back(i);
                    ret = nex;
                    pos = par[pos];
                    continue;
                }
                done[pos] = true;
                st.push_back(i);
                par[nex] = pos;
                pos = nex;
            }
        }
        
        if(ret != -1){
            reverse(ans.begin(), ans.end());
            return ans;
        }
    }
    
    return {-1};
};
