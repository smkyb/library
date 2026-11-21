auto enumerate_combinations = [](int n, int k) {
    vector<vector<int>> res;
    if(k < 0 || k > n) return res;
    vector<int> now(k);
    iota(now.begin(), now.end(), 0);
    while(true){
        {
            res.emplace_back(now);
        }
        int ind = k-1;
        while(ind >= 0 && now[ind] == ind+n-k) --ind;
        if(ind == -1) break;
        ++now[ind];
        for(int i = ind+1; i < k; ++i) now[i] = now[i-1] + 1;
    }
    return res;
};