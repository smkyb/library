pair<long long, vector<int>> tree_diameter(int n, vector<pair<int, pair<int, long long>>> e) {
    struct _Dijkstra { private: struct graph { vector<pair<int, long long>> G; vector<int> idx; struct ref_t { vector<pair<int, long long>>::iterator begin_, end_; auto begin() const noexcept {return begin_;} auto end() const noexcept {return end_;} auto size() const noexcept {return end_ - begin_;} pair<int, long long> operator[](int p) const {return begin_[p];} }; graph() = default; graph(int n, const vector<pair<int, pair<int, long long>>> &E){ build(n, E); } void build(int n, const vector<pair<int, pair<int, long long>>> &E){ G.resize(E.size()); idx.resize(n+1); for(auto &[u, v] : E){ idx[u+1]++; } for(int i = 1; i <= n; i++) idx[i] += idx[i-1]; auto C = idx; for(auto &[u, v] : E){ G[C[u]++] = v; } } ref_t operator[](int p) {return {G.begin()+idx[p], G.begin()+idx[p+1]};} }; struct _comp { bool operator()(const pair<int, long long> &l, const pair<int, long long> &r) {return l.second > r.second;} }; public: pair<vector<long long>, vector<int>> operator()(int n, const vector<pair<int, pair<int, long long>>> &E, const vector<int> &s){ graph G(n, E); return (*this)(n, G, s); } pair<vector<long long>, vector<int>> operator()(int n, graph &G, const vector<int> &s){ vector<long long> dist(n, (1ll<<60)); vector<int> path(n, -1); priority_queue<pair<int, long long>, vector<pair<int, long long>>, _comp> Q; for(int i : s){ dist[i] = 0; Q.emplace(i, 0); } while(!Q.empty()){ auto [pos, d] = Q.top(); Q.pop(); if(d > dist[pos]) continue; for(auto [nex, cost] : G[pos]){ long long nex_cost = d + cost; if(dist[nex] > nex_cost){ dist[nex] = nex_cost; Q.emplace(nex, nex_cost); path[nex] = pos; } } } for(auto &i : dist) if(i >= (1ll<<60)) i = -1; return make_pair(dist, path); } } dijkstra;
    if((int)e.size() == n-1){
        for(int i = 0; i < n-1; i++) {
            auto [nex, c] = e[i].second;
            e.push_back({nex, {e[i].first, c}});
        }
    }
    auto d1 = dijkstra(n, e, {0}).first;
    int s = max_element(d1.begin(), d1.end()) - d1.begin();
    auto [d2, path] = dijkstra(n, e, {s});
    int g = max_element(d2.begin(), d2.end()) - d2.begin();
    vector<int> res;
    for(int p = g; p != -1; p = path[p]) res.push_back(p);
    reverse(res.begin(), res.end());
    return make_pair(d2[g], res);
}
