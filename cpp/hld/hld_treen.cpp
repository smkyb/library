template<int MAXN>

struct hld_tree{
    int n;
    int dep[MAXN], in[MAXN], head[MAXN], par[MAXN];
    int node[MAXN];
    
    hld_tree(int _n, vector<int>& g, const vector<int>& start, int r = 0):
        n(_n){
            assert(0 <= r && r < n);
            
            if(ssize(g) == (n-1)*2){
                int sub_siz[MAXN], idx[MAXN+1];
                fill(sub_siz, sub_siz+n, 1);
                memcpy(idx, start.data(), sizeof(int)*start.size());
                
                int cur = r;
                par[r] = -1;
                while(true){
                    if(idx[cur] < start[cur+1]-(cur!=r)){
                        int nex = g[idx[cur]++];
                        if(nex == par[cur]){
                            g[--idx[cur]] = g[start[cur+1]-1];
                            continue;
                        }
                        par[nex] = cur;
                        cur = nex;
                    } else {
                        if(cur == r) break;
                        sub_siz[par[cur]] += sub_siz[cur];
                        cur = par[cur];
                    }
                }
                
                memcpy(idx, start.data(), sizeof(int)*start.size());
                int state[MAXN]{}, heavy[MAXN];
                
                int node_siz = 0;
                cur = r;
                head[cur] = r;
                dep[cur] = 0;
                while(1){
                    if(state[cur] == 0){
                        in[cur] = node_siz++;
                        heavy[cur] = -1;
                        for(int i = start[cur]; i < start[cur+1]-(cur!=r); i++){
                            int nex = g[i];
                            if(heavy[cur] == -1 || sub_siz[heavy[cur]] < sub_siz[nex]){
                                heavy[cur] = nex;
                            }
                        }
                        if(heavy[cur] != -1){
                            int nex = heavy[cur];
                            dep[nex] = dep[cur];
                            head[nex] = head[cur];
                            state[cur] = 2;
                            cur = nex;
                        } else state[cur] = 2;
                    } else if(state[cur] == 2){
                        if(idx[cur] < start[cur+1]-(cur!=r)){
                            int nex = g[idx[cur]++];
                            if(nex == heavy[cur]) continue;
                            dep[nex] = dep[cur]+1;
                            head[nex] = nex;
                            cur = nex;
                        } else state[cur] = 3;
                    } else {
                        if(cur == r) break;
                        cur = par[cur];
                    }
                }
            } else if(ssize(g) == (n-1)){
                int sub_siz[MAXN], idx[MAXN];
                fill(sub_siz, sub_siz+n, 1);
                memcpy(idx, start.data(), sizeof(int)*start.size());
                
                int cur = r;
                par[r] = -1;
                while(true){
                    if(idx[cur] < start[cur+1]){
                        int nex = g[idx[cur]++];
                        par[nex] = cur;
                        cur = nex;
                    } else {
                        if(cur == r) break;
                        sub_siz[par[cur]] += sub_siz[cur];
                        cur = par[cur];
                    }
                }
                
                memcpy(idx, start.data(), sizeof(int)*start.size());
                int state[MAXN]{}, heavy[MAXN];
                
                int node_siz = 0;
                cur = r;
                head[cur] = r;
                dep[cur] = 0;
                while(1){
                    if(state[cur] == 0){
                        in[cur] = node_siz++;
                        heavy[cur] = -1;
                        for(int i = start[cur]; i < start[cur+1]; i++){
                            int nex = g[i];
                            if(heavy[cur] == -1 || sub_siz[heavy[cur]] < sub_siz[nex]){
                                heavy[cur] = nex;
                            }
                        }
                        if(heavy[cur] != -1){
                            int nex = heavy[cur];
                            dep[nex] = dep[cur];
                            head[nex] = head[cur];
                            state[cur] = 2;
                            cur = nex;
                        } else state[cur] = 2;
                    } else if(state[cur] == 2){
                        if(idx[cur] < start[cur+1]){
                            int nex = g[idx[cur]++];
                            if(nex == heavy[cur]) continue;
                            dep[nex] = dep[cur]+1;
                            head[nex] = nex;
                            cur = nex;
                        } else state[cur] = 3;
                    } else {
                        if(cur == r) break;
                        cur = par[cur];
                    }
                }
            } else assert(0);
            for(int i = 0; i < n; i++) node[in[i]] = i;
        }
    
        int prod(int l, int r, int k){
            int lc = lca(l, r);
            int dl = dist(l, lc), dr = dist(lc, r);
            if(dl+dr < k) return -1;
            if(dl >= k){
                while(true){
                    k -= in[l] - in[head[l]];
                    if(k <= 0) return node[in[head[l]]-k];
                    k--; l = par[head[l]];
                }
            } else {
                k -= dl;
                k = dr - k;
                while(true){
                    k -= in[r] - in[head[r]];
                    if(k <= 0) return node[in[head[r]]-k];
                    k--; r = par[head[r]];
                }
            }
        }
    
    int lca(int l, int r){
        assert(0 <= l && 0 <= r && l < n && r < n);
        while(true){
            if(dep[l] < dep[r]) swap(l, r);
            if(dep[l] != dep[r] || head[l] != head[r]){
                l = par[head[l]];
            } else {
                if(in[l] > in[r]) swap(l, r);
                return l;
            }
        }
    }
    
    int dist(int l, int r){
        assert(0 <= l && 0 <= r && l < n && r < n);
        int d = 0;
        while(true){
            if(dep[l] < dep[r]) swap(l, r);
            if(dep[l] != dep[r] || head[l] != head[r]){
                d += in[l] - in[head[l]] + 1;
                l = par[head[l]];
            } else {
                if(in[l] > in[r]) swap(l, r);
                d += in[r] - in[l];
                return d;
            }
        }
    }
};