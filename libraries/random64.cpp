mt19937_64 engine([]{
    random_device rd;
    auto t1 = static_cast<unsigned long long>(chrono::steady_clock::now().time_since_epoch().count());
    auto t2 = static_cast<unsigned long long>(chrono::high_resolution_clock::now().time_since_epoch().count());
    seed_seq seq{
        rd(), rd(), rd(), rd(),
        static_cast<unsigned>(t1), static_cast<unsigned>(t1 >> 32),
        static_cast<unsigned>(t2), static_cast<unsigned>(t2 >> 32)
    };
    return mt19937_64(seq);
}());