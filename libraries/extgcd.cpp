// ax + by = gcd(|a|, |b|)
pair<long long, long long> extgcd(long long a, long long b) {
    long long sa = (a < 0 ? -1 : 1), sb = (b < 0 ? -1 : 1);
    unsigned long long ua = (a < 0 ? 0ULL - static_cast<unsigned long long>(a) : static_cast<unsigned long long>(a));
    unsigned long long ub = (b < 0 ? 0ULL - static_cast<unsigned long long>(b) : static_cast<unsigned long long>(b));
    long long x0 = 1, y0 = 0, x1 = 0, y1 = 1;
    while (ub != 0) {
        unsigned long long q = ua / ub, r = ua % ub;
        ua = ub; ub = r;
        if (r == 0) {
            x0 = x1; y0 = y1;
            break;
        }
        long long nx = x0 - static_cast<long long>(q) * x1;
        long long ny = y0 - static_cast<long long>(q) * y1;
        x0 = x1; y0 = y1;
        x1 = nx; y1 = ny;
    }
    return {x0 * sa, y0 * sb};
}
