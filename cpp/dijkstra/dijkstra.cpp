struct _Dijkstra {
    private:
    struct graph { vector<pair<int, long long>> G; vector<int> idx; struct ref_t { vector<pair<int, long long>>::iterator begin_, end_; auto begin() const noexcept {return begin_;} auto end() const noexcept {return end_;} auto size() const noexcept {return end_ - begin_;} pair<int, long long> operator[](int p) const {return begin_[p];} }; graph() = default; graph(int n, const vector<pair<int, pair<int, long long>>> &E){ build(n, E); } void build(int n, const vector<pair<int, pair<int, long long>>> &E){ G.resize(E.size()); idx.resize(n+1); for(auto &[u, v] : E){ idx[u+1]++; } for(int i = 1; i <= n; i++) idx[i] += idx[i-1]; auto C = idx; for(auto &[u, v] : E){ G[C[u]++] = v; } } ref_t operator[](int p) {return {G.begin()+idx[p], G.begin()+idx[p+1]};} };
    struct _comp { bool operator()(const pair<int, long long> &l, const pair<int, long long> &r) {return l.second > r.second;} };
    public:
    //一般呼び出し用
    pair<vector<long long>, vector<int>> operator()(int n, const vector<pair<int, pair<int, long long>>> &E, const vector<int> &s){
        graph G(n, E);
        return (*this)(n, G, s);
    }
    //「自分用に」graph構造体でも呼び出せるようにしているので, これはREADMEに書かなくても良い
    template<typename graphlike, typename = enable_if_t<!is_same_v<decay_t<graphlike>, vector<pair<int, pair<int, long long>>>>>>
    pair<vector<long long>, vector<int>> operator()(int n, graphlike &G, const vector<int> &s){
        vector<long long> dist(n, (1ll<<60));
        vector<int> path(n, -1);
        priority_queue<pair<int, long long>, vector<pair<int, long long>>, _comp> Q;
        for(int i : s){
            dist[i] = 0;
            Q.emplace(i, 0);
        }
        while(!Q.empty()){
            auto [pos, d] = Q.top();
            Q.pop();
            if(d > dist[pos]) continue;
            for(auto [nex, cost] : G[pos]){
                long long nex_cost = d + cost;
                if(dist[nex] > nex_cost){
                    dist[nex] = nex_cost;
                    Q.emplace(nex, nex_cost);
                    path[nex] = pos;
                }
            }
        }
        for(auto &i : dist) if(i >= (1ll<<60)) i = -1;
        return make_pair(dist, path);
    }
} dijkstra;