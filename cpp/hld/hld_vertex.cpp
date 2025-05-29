template<typename S, auto op, auto e = []{return S();}, typename F = int, auto mapping = []{}, auto composition = []{}, auto id = []{}>
struct hld{
    using node_type = segtree<S, op, e>;
    //using node_type = lazy_segtree<S, op, e, F, mapping, composition, id>
    
    int *dum0, *dum1;
    int n, r, edge_idx;
    int *dep, *in, *out, *head, *par;
    int *E;
    
    node_type node[2];
    
    hld(int _n, int _r = -1):
        n(_n), r(_r), edge_idx(0){
            if(r == -1) r = random_device()()%n;
            dum0 = new int[n*7-2];
            dep = dum0;
            in = dum0+n;
            out = dum0+n*2;
            head = dum0+n*3;
            par = dum0+n*4;
            E = dum0+n*5;
        }
    
    void add_edge(int u, int v){
        E[edge_idx++] = u;
        E[edge_idx++] = v;
    }
    
    void build(const vector<S>& v){
        dum1 = new int[n*8+1];
        int *g = dum1;
        int *start = dum1+n*2-2;
        int *C = dum1+n*3-1;
        memset(start, 0, sizeof(int)*(n+1));
        
        for(int i = 0; i < 2*n-2; i+=2){
            start[E[i]+1]++;
            start[E[i+1]+1]++;
        }
        for(int i = 1; i <= n; i++) start[i] += start[i-1];
        memcpy(C, start, sizeof(int)*(n+1));
        
        for(int i = 0; i < 2*n-2; i+=2){
            g[C[E[i]]++] = E[i+1];
            g[C[E[i+1]]++] = E[i];
        }
        
        int *sub_siz = dum1+n*4;
        int *idx = dum1+n*5;
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
        int *state = dum1+n*6+1;
        int *heavy = dum1+n*7+1;
        memset(state, 0, sizeof(int)*n);
        
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
        
        node[0] = node_type(node_v);
        reverse(node_v.begin(), node_v.end());
        node[1] = node_type(node_v);
        
        delete[] dum1;
    }
    
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
    
    void add(int p, const S& x){
        assert(0 <= p && p < n);
        node[0].add(in[p], x);
        node[1].add(n-in[p]-1, x);
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
};
