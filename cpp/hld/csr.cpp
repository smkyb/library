void read_csr_ud_graph(vector<int>& g, vector<int>& start, int m, int num = -1){
    assert(ssize(g) == 2*m);
    int n = ssize(start)-1;
    
    vector<array<int, 2>> E(m);
    for(int i = 0; i < m; i++){
        int u, v;
        cin >> u >> v;
        u += num; v += num;
        start[u+1]++;
        start[v+1]++;
        E[i][0] = u;
        E[i][1] = v;
    }
    for(int i = 1; i <= n; i++) start[i] += start[i-1];
    auto C = start;
    for(int i = 0; i < m; i++){
        auto [u, v] = E[i];
        g[C[u]++] = v;
        g[C[v]++] = u;
    }
}
void read_csr_d_graph(vector<int>& g, vector<int>& start, int m, int num = -1){
    assert(ssize(g) == m);
    int n = ssize(start)-1;
    
    vector<array<int, 2>> E(m);
    for(int i = 0; i < m; i++){
        int u, v;
        cin >> u >> v;
        u += num; v += num;
        start[u+1]++;
        E[i][0] = u;
        E[i][1] = v;
    }
    for(int i = 1; i <= n; i++) start[i] += start[i-1];
    auto C = start;
    for(int i = 0; i < m; i++){
        auto [u, v] = E[i];
        g[C[u]++] = v;
    }
}