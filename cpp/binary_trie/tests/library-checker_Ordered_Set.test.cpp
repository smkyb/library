#define PROBLEM "https://judge.yosupo.jp/problem/ordered_set"

#include "../../../_initialize.cpp"
#include "../binary_trie.cpp"

int main(){
    int n, q;
    cin >> n >> q;
    
    binary_trie<uint> S;
    for(int i = 0; i < n; i++){
        int a;
        cin >> a;
        S.insert(a);
    }
    
    while(q--){
        int t, x;
        cin >> t >> x;
        if(t == 0){
            S.insert(x);
        } elif(t == 1){
            S.erase_all(x);
        } elif(t == 2){
            x--;
            if(x < S.size()) cout << S[x] << el;
            else cout << -1 << el;
        } elif(t == 3){
            cout << S.order(x+1) << el;
        } elif(t == 4){
            auto ans = S.less_bound(x);
            if(ans.exist) cout << ans.val << el;
            else cout << -1 << el;
        } elif(t == 5){
            auto ans = S.lower_bound(x);
            if(ans.exist) cout << ans.val << el;
            else cout << -1 << el;
        }
    }
}
