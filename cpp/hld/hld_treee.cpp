//非可換なら
#define HLD_NON_COMMUTATIVE false
//addが使えるなら
#define HLD_HAS_OPERATION_ADD false
//区間作用があるなら
#define HLD_HAS_LAZY_APPLY false

template<int MAXN, typename S, auto op, auto e>

struct hld_tree{
    using node_type = sparse_table<S, op, e>;
    
    int n;
    int dep[MAXN], in[MAXN], out[MAXN], head[MAXN], par[MAXN];
    
    #if HLD_NON_COMMUTATIVE
    node_type node[2];
    #else
    node_type node;
    #endif
    
    hld_tree(int _n, vector<pair<S, int>>& g, const vector<int>& start, int r = 0):
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
                        int nex = g[idx[cur]++].second;
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
                            int nex = g[i].second;
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
                            int nex = g[idx[cur]++].second;
                            if(nex == heavy[cur]) continue;
                            dep[nex] = dep[cur]+1;
                            head[nex] = nex;
                            cur = nex;
                        } else state[cur] = 3;
                    } else {
                        out[cur] = node_siz;
                        if(cur == r) break;
                        cur = par[cur];
                    }
                }
                
                vector<S> node_v(n);
                for(int i = 0; i < n; i++){
                    for(int j = start[i]; j < start[i+1]-(i!=r); j++){
                        auto& [w, to] = g[j];
                        node_v[in[to]] = w;
                    }
                }
                
                #if HLD_NON_COMMUTATIVE
                node[0] = node_type(node_v);
                reverse(node_v.begin(), node_v.end());
                node[1] = node_type(node_v);
                #else
                node = node_type(node_v);
                #endif
            } else if(ssize(g) == (n-1)){
                int sub_siz[MAXN], idx[MAXN];
                fill(sub_siz, sub_siz+n, 1);
                memcpy(idx, start.data(), sizeof(int)*start.size());
                
                int cur = r;
                par[r] = -1;
                while(true){
                    if(idx[cur] < start[cur+1]){
                        int nex = g[idx[cur]++].second;
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
                            int nex = g[i].second;
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
                            int nex = g[idx[cur]++].second;
                            if(nex == heavy[cur]) continue;
                            dep[nex] = dep[cur]+1;
                            head[nex] = nex;
                            cur = nex;
                        } else state[cur] = 3;
                    } else {
                        out[cur] = node_siz;
                        if(cur == r) break;
                        cur = par[cur];
                    }
                }
                
                vector<S> node_v(n);
                for(int i = 0; i < n; i++){
                    for(int j = start[i]; j < start[i+1]; j++){
                        auto& [w, to] = g[j];
                        node_v[in[to]] = w;
                    }
                }
                
                #if HLD_NON_COMMUTATIVE
                node[0] = node_type(node_v);
                reverse(node_v.begin(), node_v.end());
                node[1] = node_type(node_v);
                #else
                node = node_type(node_v);
                #endif
            } else assert(0);
        }
    
    #if HLD_NON_COMMUTATIVE
    
    S prod(int l, int r){
        assert(0 <= l && 0 <= r && l < n && r < n);
        S l_ans = e();
        S r_ans = e();
        bool rev = false;
        while(true){
            if(dep[l] < dep[r]) swap(l, r), rev ^= 1;
            if(dep[l] != dep[r] || head[l] != head[r]){
                if(!rev) l_ans = op(l_ans, node[1].prod(n-in[l]-1, n-in[head[l]]));
                else r_ans = op(node[0].prod(in[head[l]], in[l]+1), r_ans);
                l = par[head[l]];
            } else {
                if(in[l] > in[r]) swap(l, r), rev ^= 1;
                if(!rev) l_ans = op(l_ans, node[0].prod(in[l], in[r]+1));
                else r_ans = op(node[1].prod(n-in[r]-1, n-in[l]), r_ans);
                return op(l_ans, r_ans);
            }
        }
    }
    
    S prod(int r){
        assert(0 <= r && r < n);
        return node[0].prod(in[r], out[r]);
    }
    
    #else
    
    S prod(int l, int r){
        assert(0 <= l && 0 <= r && l < n && r < n);
        S ans = e();
        while(true){
            if(dep[l] < dep[r]) swap(l, r);
            if(dep[l] != dep[r] || head[l] != head[r]){
                ans = op(ans, node.prod(in[head[l]], in[l]+1));
                l = par[head[l]];
            } else {
                if(in[l] > in[r]) swap(l, r);
                if(l != r) ans = op(ans, node.prod(in[l]+1, in[r]+1));
                return ans;
            }
        }
    }
    
    S prod(int r){
        assert(0 <= r && r < n);
        return node.prod(in[r], out[r]);
    }
    
    #endif
    
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
    
    #if HLD_HAS_LAZY_APPLY
    
    #if HLD_NON_COMMUTATIVE
    
    void apply(int l, int r, const F& x){
        while(true){
            if(dep[l] < dep[r]) swap(l, r);
            if(dep[l] != dep[r] || head[l] != head[r]){
                node[0].apply(in[head[l]], in[l]+1, x);
                node[1].apply(n-in[l]-1, n-in[head[l]], x);
                l = par[head[l]];
            } else {
                if(in[l] > in[r]) swap(l, r);
                if(l != r) node[0].apply(in[l]+1, in[r]+1, x);
                if(l != r) node[1].apply(n-in[r]-1, n-in[l]-1, x);
                break;
            }
        }
    }
    
    void apply(int r, const F& x){
        node[0].apply(in[r]+1, out[r], x);
        node[1].apply(n-out[r], n-in[r]-1, x);
    }
    
    #else
    
    void apply(int l, int r, const F& x){
        while(true){
            if(dep[l] < dep[r]) swap(l, r);
            if(dep[l] != dep[r] || head[l] != head[r]){
                node.apply(in[head[l]], in[l]+1, x);
                l = par[head[l]];
            } else {
                if(in[l] > in[r]) swap(l, r);
                if(l != r) node.apply(in[l]+1, in[r]+1, x);
                break;
            }
        }
    }
    
    void apply(int r, const F& x){
        node.apply(in[r]+1, out[r], x);
    }
    
    #endif
    #endif
};