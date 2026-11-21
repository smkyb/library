// returns 2 * area (when is_signed = true, CCW(1, 2, 3) > 0)
// 注意: 整数入力時は座標差と結果(2*area)が long long 範囲内に収まること (__int128 非対応環境では途中積も long long)
auto triangle_area2 = [](auto x1, auto y1, auto x2, auto y2, auto x3, auto y3, bool is_signed = false){
    using C = common_type_t<decltype(x1), decltype(y1), decltype(x2), decltype(y2), decltype(x3), decltype(y3)>;
    using R = conditional_t<is_floating_point_v<C>, C, conditional_t<(sizeof(C) > sizeof(long long)), C, long long>>;
    R dx1 = static_cast<R>(x2) - static_cast<R>(x1);
    R dy1 = static_cast<R>(y2) - static_cast<R>(y1);
    R dx2 = static_cast<R>(x3) - static_cast<R>(x1);
    R dy2 = static_cast<R>(y3) - static_cast<R>(y1);
    R res;
#ifdef __SIZEOF_INT128__
    if constexpr (!is_floating_point_v<R> && sizeof(R) <= sizeof(long long)) {
        res = static_cast<R>(static_cast<__int128>(dx1) * dy2 - static_cast<__int128>(dy1) * dx2);
    } else {
        res = dx1 * dy2 - dy1 * dx2;
    }
#else
    res = dx1 * dy2 - dy1 * dx2;
#endif
    if(!is_signed && res < 0) res = -res;
    return res;
};