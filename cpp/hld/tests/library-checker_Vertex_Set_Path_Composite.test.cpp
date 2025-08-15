#define PROBLEM "https://judge.yosupo.jp/problem/vertex_set_path_composite"

#include "../../../_initialize.cpp"
#include "../../segtree/segtree.cpp"
#include "../hld.cpp"
#include "../../../atcoder/modint"
using mint9 = atcoder::modint998244353;

using S = pair<mint9, mint9>;
S op(const S& l, const S& r) {return {r.first*l.first, r.first*l.second + r.second};}
S e() {return {1, 0};}

signed main(){
    int n, q;
    cin >> n >> q;
    vector<S> A(n);
    for(int i = 0; i < n; i++){
        int a, b;
        cin >> a >> b;
        A[i] = {a, b};
    }
    hld<segtree<S, op, e>> tree(n);
    
    for(int i = 0; i < n-1; i++){
        int u, v;
        cin >> u >> v;
        tree.add_edge(u, v);
    }
    
    tree.build(A);
    
    while(q--){
        int t, p, c, d;
        cin >> t >> p >> c >> d;
        if(t == 0) tree.set(p, {c, d});
        else{
            S ans = tree.prod(p, c);
            cout << ans.first*d + ans.second << el;
        }
    }
}
