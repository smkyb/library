// 注意: 長方形 2次元配列をその場で破壊的に時計回り 90度回転する
auto rotate_matrix = [](auto& mat){
    int h = mat.size();
    if(h == 0) return;
    int w = mat[0].size();
    using Mat = remove_reference_t<decltype(mat)>;
    using Row = typename Mat::value_type;
    Mat res(w, Row(h, typename Row::value_type{}));
    for(int i = 0; i < h; i++) for(int j = 0; j < w; j++) res[j][h-1-i] = mat[i][j];
    mat = move(res);
};