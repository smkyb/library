// 注意: 非負 long long 重み・0-indexed 必須; 戻り値は {dist, prev}
//       未到達は dist[v] == -1 (prev 復元時の終端も -1); 距離が LLONG_MAX を超える経路は緩和しない
struct _Dijkstra {
    struct _csr {
        vector<pair<int, long long>> G;
        vector<int> idx;
        struct ref_t {
            vector<pair<int, long long>>::const_iterator begin_, end_;
            auto begin() const noexcept {return begin_;}
            auto end() const noexcept {return end_;}
            auto size() const noexcept {return end_ - begin_;}
            const pair<int, long long>& operator[](int p) const {return begin_[p];}
        };
        _csr() = default;
        _csr(int n, const vector<pair<int, pair<int, long long>>> &E){ build(n, E); }
        void build(int n, const vector<pair<int, pair<int, long long>>> &E){
            assert(n >= 0);
            G.resize(E.size());
            idx.assign(n + 1, 0);
            for(const auto &[u, v] : E){
                assert(0 <= u && u < n && 0 <= v.first && v.first < n);
                idx[u + 1]++;
            }
            for(int i = 1; i <= n; ++i) idx[i] += idx[i - 1];
            for(const auto &[u, v] : E){
                G[idx[u]++] = v;
            }
            for(int i = n; i > 0; --i) idx[i] = idx[i - 1];
            idx[0] = 0;
        }
        ref_t operator[](int p) const {return {G.cbegin()+idx[p], G.cbegin()+idx[p+1]};}
    };
    struct MinHeap4 {
        vector<pair<unsigned long long, int>> h;
        bool empty() const noexcept { return h.empty(); }
        void emplace(unsigned long long d, int v){
            size_t i = h.size();
            h.emplace_back(d, v);
            while(i > 0){
                size_t p = (i - 1) >> 2;
                if(h[p].first <= d) break;
                h[i] = h[p];
                i = p;
            }
            h[i] = {d, v};
        }
        pair<unsigned long long, int> pop_min(){
            auto ret = h[0];
            auto x = h.back();
            h.pop_back();
            if(!h.empty()){
                size_t n = h.size(), i = 0;
                while(true){
                    size_t c = (i << 2) + 1;
                    if(c >= n) break;
                    size_t best = c;
                    size_t end = min(c + 4, n);
                    for(size_t k = c + 1; k < end; ++k){
                        if(h[k].first < h[best].first) best = k;
                    }
                    if(h[best].first >= x.first) break;
                    h[i] = h[best];
                    i = best;
                }
                h[i] = x;
            }
            return ret;
        }
    };
    pair<vector<long long>, vector<int>> operator()(int n, const vector<pair<int, pair<int, long long>>> &edges, const vector<int> &starts){
        _csr adj(n, edges);
        return (*this)(n, adj, starts);
    }
    template<typename Graph, typename = enable_if_t<!is_same_v<decay_t<Graph>, vector<pair<int, pair<int, long long>>>>>>
    pair<vector<long long>, vector<int>> operator()(int n, Graph &&graph, const vector<int> &starts){
        assert(n >= 0);
        constexpr unsigned long long MAX_D = static_cast<unsigned long long>(numeric_limits<long long>::max());
        vector<long long> dist(n, -1);
        vector<int> path(n, -1);
        MinHeap4 Q;
        for(int i : starts){
            assert(0 <= i && i < n);
            if(dist[i] == 0) continue;
            dist[i] = 0;
            Q.emplace(0ULL, i);
        }
        while(!Q.empty()){
            auto [d, pos] = Q.pop_min();
            if(d > static_cast<unsigned long long>(dist[pos])) continue;
            for(auto [nex, cost] : graph[pos]){
                assert(0 <= nex && nex < n && cost >= 0);
                unsigned long long uc = static_cast<unsigned long long>(cost);
                if(uc <= MAX_D - d){
                    unsigned long long nd = d + uc;
                    if(nd < static_cast<unsigned long long>(dist[nex])){
                        dist[nex] = static_cast<long long>(nd);
                        Q.emplace(nd, nex);
                        path[nex] = pos;
                    }
                }
            }
        }
        return make_pair(dist, path);
    }
} dijkstra;