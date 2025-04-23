//非可換なら
#define HLD_NON_COMMUTATIVE false
//addが使えるなら
#define HLD_HAS_OPERATION_ADD true
//区間作用があるなら
#define HLD_HAS_LAZY_APPLY false

template<int MAXN, typename S, auto op, auto e>
//template<int MAXN, typename S, auto op, auto e, typename F, auto mapping, auto composition, auto id>

struct hld_tree{
    using node_type = fenwick_tree<S>;
    //using node_type = lazy_segtree<S, op, e, F, mapping, composition, id>
    
    int n, r, edge_idx;
    int dep[MAXN], in[MAXN], out[MAXN], head[MAXN], par[MAXN];
    array<int, 2> E[MAXN-1];
    
    #if HLD_NON_COMMUTATIVE
    node_type node[2];
    #else
    node_type node;
    #endif
    
    hld_tree(int _n, int _r = -1):
        n(_n), r(_r), edge_idx(0){
            if(r == -1) r = random_device()()%n;
        }
    
    void add_edge(int u, int v){
        E[edge_idx][0] = u;
        E[edge_idx][1] = v;
        edge_idx++;
    }
    
    void build(const vector<S>& v){
        int g[(MAXN-1)*2], start[MAXN+1], C[MAXN+1];
        for(int i = 0; i < n-1; i++){
            auto [u, v] = E[i];
            start[u+1]++;
            start[v+1]++;
        }
        for(int i = 1; i <= n; i++) start[i] += start[i-1];
        memcpy(C, start, sizeof(int)*(n+1));
        for(int i = 0; i < n-1; i++){
            auto [u, v] = E[i];
            g[C[u]++] = v;
            g[C[v]++] = u;
        }
        
        int sub_siz[MAXN], idx[MAXN+1];
        fill(sub_siz, sub_siz+n, 1);
        memcpy(idx, start, sizeof(int)*(n+1));
        
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
        
        memcpy(idx, start, sizeof(int)*(n+1));
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
                out[cur] = node_siz;
                if(cur == r) break;
                cur = par[cur];
            }
        }
        
        vector<S> node_v(n);
        for(int i = 0; i < n; i++) node_v[in[i]] = v[i];
        
        #if HLD_NON_COMMUTATIVE
        node[0] = node_type(node_v);
        reverse(node_v.begin(), node_v.end());
        node[1] = node_type(node_v);
        #else
        node = node_type(node_v);
        #endif
    }
    
    #if HLD_HAS_OPERATION_ADD
    
    #if HLD_NON_COMMUTATIVE
    
    void add(int p, const S& x){
        assert(0 <= p && p < n);
        node[0].add(in[p], x);
        node[1].add(n-in[p]-1, x);
    }
    
    #else
    
    void add(int p, const S& x){
        assert(0 <= p && p < n);
        node.add(in[p], x);
    }
    
    #endif
    #endif
    
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
    
    S get(int p){
        assert(0 <= p && p < n);
        return node[0].get(in[p]);
    }
    
    void set(int p, const S& x){
        assert(0 <= p && p < n);
        node[0].set(in[p], x);
        node[1].set(n-in[p]-1, x);
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
                ans = op(ans, node.prod(in[l], in[r]+1));
                return ans;
            }
        }
    }
    
    S prod(int r){
        assert(0 <= r && r < n);
        return node.prod(in[r], out[r]);
    }
    
    S get(int p){
        assert(0 <= p && p < n);
        return node.get(in[p]);
    }
    
    void set(int p, const S& x){
        assert(0 <= p && p < n);
        node.set(in[p], x);
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
                node[0].apply(in[l], in[r]+1, x);
                node[1].apply(n-in[r]-1, n-in[l], x);
                break;
            }
        }
    }
    
    void apply(int r, const F& x){
        node[0].apply(in[r], out[r], x);
        node[1].apply(n-out[r], n-in[r], x);
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
                node.apply(in[l], in[r]+1, x);
                break;
            }
        }
    }
    
    void apply(int r, const F& x){
        node.apply(in[r], out[r], x);
    }
    
    #endif
    #endif
};
