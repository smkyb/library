template<typename T>
T add_sat(T a, T b){
    T MAX = numeric_limits<T>::max(), MIN = numeric_limits<T>::min();
    if(a > 0 && b > MAX - a) return MAX;
    if(a < 0 && b < MIN - a) return MIN;
    return a+b;
}
template<typename T>
T mul_sat(T a, T b){
    T MAX = numeric_limits<T>::max(), MIN = numeric_limits<T>::min();
    T res;
    if(__builtin_mul_overflow(a, b, &res)){
        if((a>0)^(b>0)) return MIN;
        else return MAX;
    }
    return a*b;
}