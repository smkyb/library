auto topological_sort = [](const auto& graph){
    int n = static_cast<int>(graph.size());
    vector<int> cnt(n);
    vector<int> res; res.reserve(n);
    queue<int> Q;
    for(int u = 0; u < n; u++) for(int v : graph[u]) {
        assert(0 <= v && v < n);
        cnt[v]++;
    }
    for(int i = 0; i < n; i++) if(cnt[i] == 0) Q.push(i);
    while(!Q.empty()){
        int pos = Q.front();
        Q.pop();
        res.push_back(pos);
        for(int i : graph[pos]) if(--cnt[i] == 0) Q.push(i);
    }
    if(static_cast<int>(res.size()) != n) return vector<int>{};
    return res;
};