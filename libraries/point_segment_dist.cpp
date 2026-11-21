// 注意: 点 (px, py) と線分 (x1, y1)-(x2, y2) の距離を返す
//       引数順: (点x, 点y, 端点1x, 端点1y, 端点2x, 端点2y)
auto point_segment_dist = [](long double px, long double py, long double x1, long double y1, long double x2, long double y2){
    px -= x1; py -= y1;
    x2 -= x1; y2 -= y1;
    long double len = x2*x2 + y2*y2;
    if(len == 0.0L) return hypot(px, py);
    long double t = (px*x2 + py*y2) / len;
    t = max(0.0L, min(1.0L, t));
    return hypot(x2*t-px, y2*t-py);
};
