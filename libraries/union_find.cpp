struct union_find{
    vector<int> d;
    int siz;
    
    union_find(int n = 0):
        d(n, -1), siz(n){}
    union_find(const vector<vector<int>>& g):
    union_find(static_cast<int>(g.size())){
        for(int i = 0; i < siz; i++){
            for(int j : g[i]) merge(i, j);
        }
    }
    
    int leader(int x){
        while(d[x] >= 0){
            if(d[d[x]] < 0) return d[x];
            x = d[x] = d[d[x]];
        }
        return x;
    }
    
    bool merge(int x, int y){
        x = leader(x);
        y = leader(y);
        if(x == y) return false;
        if(d[x] > d[y]) swap(x, y);
        d[x] += d[y];
        d[y] = x;
        return true;
    }
    
    bool same(int x, int y){
        return leader(x) == leader(y);
    }
    
    vector<vector<int>> groups(){
        vector<int> idx(siz, -1);
        vector<vector<int>> res;
        for(int i = 0; i < siz; ++i){
            int r = leader(i);
            if(idx[r] == -1){
                idx[r] = static_cast<int>(res.size());
                res.emplace_back();
                res.back().reserve(static_cast<size_t>(-d[r]));
            }
            res[idx[r]].push_back(i);
        }
        return res;
    }
};