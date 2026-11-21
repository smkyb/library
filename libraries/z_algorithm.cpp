vector<int> z_algorithm(const string& s){
    int n = s.size();
    if(n == 0) return {};
    vector<int> res(n);
    res[0] = n;
    int l = 1, w = 0;
    while(l < n){
        while(l+w < n && s[l+w] == s[w]) w++;
        res[l] = w;
        w = max(w, 1);
        int k = 1;
        if(l+1 != n) while(k+res[k] < w){
            res[l+k] = res[k];
            k++;
        }
        l += k;
        w -= k;
    }
    return res;
}