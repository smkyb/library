struct unionfind{
    vector<int> par, rank;
    int siz;
    
    unionfind(int n):
        par(n), rank(n, 1), siz(n){
            iota(par.begin(), par.end(), 0);
        }
    unionfind(const vector<vector<int>>& v):
    unionfind(v.size()){
        for(int i = 0; i < siz; i++){
            for(int j : v[i]) merge(i, j);
        }
    }
    
    int root(int x){
        if(par[x] == x) return x;
        return par[x] = root(par[x]);
    }
    
    bool merge(int x, int y){
        x = root(x);
        y = root(y);
        if(x == y) return false;
        if(rank[x] < rank[y]) swap(x, y);
        par[y] = x;
        rank[x] += rank[y];
        return true;
    }
    
    bool same(int x, int y){
        return root(x) == root(y);
    }
    
    vector<vector<int>> groups(){
        vector<vector<int>> res_memo(siz);
        for(int i = 0; i < siz; i++){
            res_memo[root(i)].emplace_back(i);
        }
        vector<vector<int>> res;
        for(int i = 0; i < siz; i++){
            if(!res_memo[i].empty()) res.emplace_back(res_memo[i]);
        }
        return res;
    }
};
