#define PROBLEM "https://judge.yosupo.jp/problem/point_set_range_sort_range_composite"

#include "../../../_initialize.cpp"

struct S{
    mint9 first, second;
    S() : first(mint9::raw(1)), second(mint9::raw(0)) {}
    S(const S &o) = default;
    S(const mint9 &l, const mint9 &r) : first(l), second(r) {}
    static S e() {return {1, 0};}
    static S op(const S &l, const S &r) {return {r.first*l.first, r.first*l.second + r.second};}
};

int main(){
    uint n, q;
    cin >> n >> q;
    vector<pair<uint, S>> A(n);
    for(uint i = 0; i < n; i++){
        uint a, b;
        cin >> A[i].first >> a >> b;
        A[i].second = {a, b};
    }
    sort_segtree<uint, S, S::op, S::e> trie(A);
    
    uint i{}, p{}, a{}, b{}, l{}, r{}, x{};
    S res{};
    while(q--){
        uint t;
        cin >> t;
        switch(t){
            case 0:
                cin >> i >> p >> a >> b;
                trie.set(i, p, {a, b});
                break;
            case 1:
                cin >> l >> r >> x;
                res = trie.prod(l, r);
                cout << (res.first*x + res.second).val() << '\n';
                break;
            case 2:
                cin >> l >> r;
                trie.sort(l, r);
                break;
            case 3:
                cin >> l >> r;
                trie.sort_rev(l, r);
                break;
        }
    }
}
