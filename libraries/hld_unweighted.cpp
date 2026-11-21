// 注意: 重みなし HLD (0-indexed・連結な木・C++20); 0 < n <= max_n (固定配列サイズに注意)
//       CSR 引数 edges は無向 (サイズ 2*(n-1)) または根から子向きの有向 (サイズ n-1) のいずれか
//       jump(u, v, k) はパス上の 0-indexed k 番目の頂点 (範囲外は -1)
template<int max_n>
struct hld_unweighted{
    struct Chain{
        int head, jump;
    };
    
    int n;
    int depth[max_n], in[max_n], node[max_n];
    Chain chain[max_n];
    
    hld_unweighted(int n, const vector<int>& edges, const vector<int>& start, int root = 0):
        n(n){
        assert(0 < n && n <= max_n);
        assert(0 <= root && root < n);
        bool is_undir = (ssize(edges) == (n - 1) * 2);
        assert(is_undir || ssize(edges) == n - 1);
        
        vector<int> ord, par(n, -1);
        ord.reserve(n);
        ord.push_back(root);
        depth[root] = 0;
        for(int idx = 0; idx < (int)ord.size(); ++idx){
            int u = ord[idx];
            int pu = par[u];
            int du = depth[u] + 1;
            int lim = start[u + 1];
            for(int i = start[u]; i < lim; ++i){
                int v = edges[i];
                if(is_undir && v == pu) continue;
                par[v] = u;
                depth[v] = du;
                ord.push_back(v);
            }
        }
        assert((int)ord.size() == n);
        
        vector<int> sub_siz(n, 1), heavy(n, -1), heavy_sz(n, 0);
        for(int k = n - 1; k >= 1; --k){
            int u = ord[k];
            int p = par[u];
            int su = sub_siz[u];
            sub_siz[p] += su;
            if(su > heavy_sz[p]){
                heavy_sz[p] = su;
                heavy[p] = u;
            }
        }
        
        vector<int> cur_pos(n);
        in[root] = 0;
        node[0] = root;
        chain[0] = {0, -1};
        cur_pos[root] = 1 + (heavy[root] != -1 ? sub_siz[heavy[root]] : 0);
        for(int k = 1; k < n; ++k){
            int u = ord[k];
            int p = par[u];
            int iu;
            if(u == heavy[p]){
                iu = in[p] + 1;
                chain[iu] = chain[in[p]];
            } else {
                iu = cur_pos[p];
                cur_pos[p] += sub_siz[u];
                chain[iu] = {iu, in[p]};
            }
            in[u] = iu;
            node[iu] = u;
            cur_pos[u] = iu + 1 + (heavy[u] != -1 ? sub_siz[heavy[u]] : 0);
        }
    }
    
    int _lca_in(int l, int r) const {
        while(chain[l].head != chain[r].head){
            if(l > r) l = chain[l].jump;
            else r = chain[r].jump;
        }
        return l < r ? l : r;
    }
    
    int jump(int u, int v, int k) const {
        assert(0 <= u && 0 <= v && u < n && v < n);
        int ui = in[u], vi = in[v];
        int lc = node[_lca_in(ui, vi)];
        int du = depth[u] - depth[lc], dv = depth[v] - depth[lc];
        if(k < 0 || du + dv < k) return -1;
        int cur = (k <= du ? ui : vi);
        int step = (k <= du ? k : du + dv - k);
        while(true){
            int len = cur - chain[cur].head;
            if(step <= len) return node[cur - step];
            step -= len + 1;
            cur = chain[cur].jump;
        }
    }
    
    int lca(int u, int v) const {
        assert(0 <= u && 0 <= v && u < n && v < n);
        return node[_lca_in(in[u], in[v])];
    }
    
    int dist(int u, int v) const {
        assert(0 <= u && 0 <= v && u < n && v < n);
        return depth[u] + depth[v] - 2 * depth[lca(u, v)];
    }
};
