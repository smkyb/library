// 注意: 0-indexed 無向グラフ; {始点頂点, 辺 ID 列} を返す (閉路なしは {-1, {}})
//       自己ループ・多重辺も閉路として検出する
pair<int, vector<int>> cycle_detection_undirected(int n, const auto &edges) {
    assert(n >= 0);
    int m = static_cast<int>(edges.size());
    vector<int> head(n + 1, 0);
    for(const auto &[u, v] : edges){
        assert(0 <= u && u < n && 0 <= v && v < n);
        head[u + 1]++;
        head[v + 1]++;
    }
    for(int i = 1; i <= n; ++i) head[i] += head[i - 1];
    vector<array<int, 2>> to(m * 2);
    vector<int> idx(head.begin(), head.begin() + n);
    int eid = 0;
    for(const auto &[u, v] : edges){
        to[idx[u]++] = {v, eid};
        to[idx[v]++] = {u, eid};
        ++eid;
    }
    if(n > 0) memcpy(idx.data(), head.data(), sizeof(int) * n);
    
    vector<uint8_t> state(n, 0);
    vector<int> par(n, -1), pe(n, -1), ans;
    for(int start = 0; start < n; ++start){
        if(state[start] != 0) continue;
        int pos = start;
        while(pos != -1){
            state[pos] = 1;
            if(idx[pos] < head[pos + 1]){
                auto [nex, id] = to[idx[pos]++];
                if(id == pe[pos]) continue;
                if(state[nex] == 2) continue;
                if(state[nex] == 1){
                    ans.push_back(id);
                    int cur = pos;
                    while(cur != nex){
                        ans.push_back(pe[cur]);
                        cur = par[cur];
                    }
                    reverse(ans.begin(), ans.end());
                    return {nex, ans};
                }
                par[nex] = pos;
                pe[nex] = id;
                pos = nex;
            } else {
                state[pos] = 2;
                pos = par[pos];
            }
        }
    }
    
    return {-1, {}};
}
