#include <bits/stdc++.h>
#include <cassert>
#include <atcoder/modint>
using namespace std;
using uint=unsigned; using ll=long long; using ull=unsigned long long; using ld=long double;
using mint1=atcoder::modint1000000007; using mint9=atcoder::modint998244353;

namespace sinit_detail {
    template<typename T, typename U>
    constexpr bool int_less(T a, U b){
        if constexpr(is_signed_v<T> == is_signed_v<U>) return a < b;
        else if constexpr(is_signed_v<T>) return a < 0 || static_cast<make_unsigned_t<T>>(a) < b;
        else return b >= 0 && a < static_cast<make_unsigned_t<U>>(b);
    }
    template<typename T, typename U>
    constexpr bool fits(U x){
        return !int_less(x, numeric_limits<T>::lowest()) && !int_less(numeric_limits<T>::max(), x);
    }
}

// 整数型が異なる場合、代入先で表現できない値には更新しない。
template<typename T, typename U=T>
inline bool chmin(T& a, const U& b){
    if constexpr(is_integral_v<T> && is_integral_v<U> && !is_same_v<T,U>){
        if(!sinit_detail::int_less(b, a) || !sinit_detail::fits<T>(b)) return false;
    } else if(!(a > b)) return false;
    a = b;
    return true;
}
template<typename T, typename U=T>
inline bool chmax(T& a, const U& b){
    if constexpr(is_integral_v<T> && is_integral_v<U> && !is_same_v<T,U>){
        if(!sinit_detail::int_less(a, b) || !sinit_detail::fits<T>(b)) return false;
    } else if(!(a < b)) return false;
    a = b;
    return true;
}

ll power(ll val,ll num,ll mod){assert(mod>0&&num>=0);if(mod==1)return 0;ull m=(ull)mod;val%=mod;if(val<0)val+=mod;ull v=(ull)val,res=1;if(m<=(1ULL<<32)){while(num){if(num&1)res=(res*v)%m;v=(v*v)%m;num>>=1;}}else{auto mul=[m](ull a,ull b)->ull{
#ifdef __SIZEOF_INT128__
return(ull)((__uint128_t)a*b%m);
#else
ull r=0;while(b){if(b&1){r+=a;if(r>=m)r-=m;}a<<=1;if(a>=m)a-=m;b>>=1;}return r;
#endif
};while(num){if(num&1)res=mul(res,v);v=mul(v,v);num>>=1;}}return(ll)res;}ostream&operator<<(ostream&ost,const mint1&x){ost<<x.val();return ost;}ostream&operator<<(ostream&ost,const mint9&x){ost<<x.val();return ost;}
struct Fastio{Fastio(){cin.tie(0)->sync_with_stdio(0);}}fastio;
#define all(x) begin(x),end(x)

int main(){
    
    
}

