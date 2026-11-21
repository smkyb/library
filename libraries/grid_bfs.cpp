// 注意: 4方向多始点 BFS; starts の座標は 0-indexed の (y, x) = (行, 列) 順; 未到達値は既定 inf (= 1e9)
auto grid_bfs = [](const auto& grid, const vector<pair<int, int>>& starts, char wall, int inf = 1'000'000'000){
    int h = grid.size();
    vector<int> dy = {0, 1, 0, -1}, dx = {1, 0, -1, 0};
    vector<vector<int>> dist;
    dist.reserve(h);
    for(int y = 0; y < h; ++y) dist.emplace_back(grid[y].size(), inf);
    auto inside = [&](int y, int x){
        return 0 <= y && y < h && 0 <= x && x < static_cast<int>(dist[y].size());
    };
    queue<pair<int, int>> Q;
    for(auto [sy, sx] : starts){
        if(!inside(sy, sx) || dist[sy][sx] != inf || grid[sy][sx] == wall) continue;
        Q.emplace(sy, sx);
        dist[sy][sx] = 0;
    }
    while(!Q.empty()){
        int y = Q.front().first, x = Q.front().second;
        Q.pop();
        for(int dk = 0; dk < (int)dy.size(); dk++){
            int ny = y+dy[dk], nx = x+dx[dk];
            if(!inside(ny, nx) || dist[ny][nx] != inf || grid[ny][nx] == wall) continue;
            dist[ny][nx] = dist[y][x]+1;
            Q.emplace(ny, nx);
        }
    }
    return dist;
};
